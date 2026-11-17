#include "config.h"

#include "../../crazypod_l10n.h"

#ifdef IPOD_6G


#include "kernel.h"

#include "../../crazypod_coverflow.h"
#include "../../crazypod_state.h"
#include "../features/now_playing/crazypod_now_playing_feature.h"
#include "../features/settings/crazypod_settings_feature.h"
#include "../navigation/crazypod_route_query.h"
#include "../presentation/crazypod_choice_overlay.h"
#include "../presentation/crazypod_overlay_glass.h"
#include "../presentation/crazypod_popup_motion.h"
#include "../shell/crazypod_app_catalog.h"
#include "../shell/crazypod_desktop_native.h"
#include "../shell/crazypod_notification.h"
#include "crazypod_choice_coordinator.h"

#define ROUTE_ACTION_DEPTH 4
#define RECEIPT_TICKS \
    ((HZ * 800 / 1000) > 0 ? (HZ * 800 / 1000) : 1)
#define ACTION_HOLD_MS 900
#define ACTION_HOLD_TICKS \
    ((HZ * ACTION_HOLD_MS / 1000) > 0 \
        ? (HZ * ACTION_HOLD_MS / 1000) : 1)

enum choice_hold_action {
    CHOICE_HOLD_NONE = 0,
    CHOICE_HOLD_BOOKMARK,
    CHOICE_HOLD_CONFIRMATION,
};

static struct crazypod_choice_coordinator_host host;
static struct route_state route_actions[ROUTE_ACTION_DEPTH];
static int route_action_depth;
static long receipt_until;
static bool receipt_refresh_route;
static enum choice_hold_action hold_action;
static long hold_until;

static struct route_state *route_action_current(void)
{
    return route_action_depth > 0
        ? &route_actions[route_action_depth - 1] : NULL;
}

static const struct crazypod_app_descriptor *main_menu_app(int id)
{
    return crazypod_app_catalog_find(
        (enum crazypod_app_id)id);
}

static void cancel_hold(bool restore)
{
    hold_action = CHOICE_HOLD_NONE;
    hold_until = 0;
    if(restore)
        crazypod_choice_overlay_cancel_hold_feedback();
}

static int choice_count(int kind_value, int id, void *context)
{
    enum crazypod_choice_kind kind = (enum crazypod_choice_kind)kind_value;
    (void)context;
    switch(kind) {
    case CRAZYPOD_CHOICE_SETTING:
        return crazypod_settings_feature_choice_count(id);
    case CRAZYPOD_CHOICE_NOW_PLAYING_THEME:
        return crazypod_now_playing_theme_choice_count();
    case CRAZYPOD_CHOICE_MAIN_MENU_ITEM_ACTIONS:
        return crazypod_apps_is_fixed((enum crazypod_app_id)id) ? 1 : 2;
    case CRAZYPOD_CHOICE_ROUTE_ACTIONS: {
        const struct route_state *state = route_action_current();
        return state != NULL && host.item_count != NULL
            ? host.item_count(state) : 0;
    }
    case CRAZYPOD_CHOICE_RECEIPT:
        return 1;
    default:
        return 0;
    }
}

static int current_index(int kind_value, int id, void *context)
{
    enum crazypod_choice_kind kind = (enum crazypod_choice_kind)kind_value;
    (void)context;
    switch(kind) {
    case CRAZYPOD_CHOICE_SETTING:
        return crazypod_settings_feature_choice_index(id);
    case CRAZYPOD_CHOICE_NOW_PLAYING_THEME: {
        int index;
        int count = crazypod_now_playing_theme_choice_count();
        for(index = 0; index < count; ++index)
            if(crazypod_now_playing_theme_choice_current(index))
                return index;
        return 0;
    }
    case CRAZYPOD_CHOICE_ROUTE_ACTIONS: {
        const struct route_state *state = route_action_current();
        return state != NULL && host.item_is_current != NULL &&
            host.item_is_current(state, state->selected) ? state->selected : -1;
    }
    case CRAZYPOD_CHOICE_MAIN_MENU_ITEM_ACTIONS:
    case CRAZYPOD_CHOICE_RECEIPT:
        return -1;
    default:
        return 0;
    }
}

static const char *choice_title(int kind_value, int id, void *context)
{
    enum crazypod_choice_kind kind = (enum crazypod_choice_kind)kind_value;
    (void)context;
    switch(kind) {
    case CRAZYPOD_CHOICE_SETTING:
        return crazypod_settings_feature_choice_item_title(id);
    case CRAZYPOD_CHOICE_NOW_PLAYING_THEME:
        return CP_TR("Themes");
    case CRAZYPOD_CHOICE_MAIN_MENU_ITEM_ACTIONS: {
        const struct crazypod_app_descriptor *app = main_menu_app(id);
        return app != NULL ? app->name : CP_TR("MAIN MENU");
    }
    case CRAZYPOD_CHOICE_ROUTE_ACTIONS: {
        const struct route_state *state = route_action_current();
        return state != NULL ? crazypod_route_query_title(state) : "";
    }
    default:
        return "";
    }
}

static const char *item_title(int kind_value, int id, int index, void *context)
{
    enum crazypod_choice_kind kind = (enum crazypod_choice_kind)kind_value;
    (void)context;
    switch(kind) {
    case CRAZYPOD_CHOICE_SETTING:
        return crazypod_settings_feature_choice_title(id, index);
    case CRAZYPOD_CHOICE_NOW_PLAYING_THEME:
        return crazypod_now_playing_theme_choice_title(index);
    case CRAZYPOD_CHOICE_MAIN_MENU_ITEM_ACTIONS: {
        enum crazypod_app_id app_id = (enum crazypod_app_id)id;
        if(crazypod_apps_is_fixed(app_id))
            return index == 0 ? CP_TR("Move") : "";
        if(index == 0)
            return crazypod_apps_is_enabled(app_id)
                ? CP_TR("Hide") : CP_TR("Show");
        return index == 1 ? CP_TR("Move") : "";
    }
    case CRAZYPOD_CHOICE_ROUTE_ACTIONS: {
        const struct route_state *state = route_action_current();
        return state != NULL && host.item_title != NULL
            ? host.item_title(state, index) : "";
    }
    default:
        return "";
    }
}

static bool item_color(int kind_value, int id, int index,
    uint32_t *color, void *context)
{
    enum crazypod_choice_kind kind = (enum crazypod_choice_kind)kind_value;
    (void)context;
    if(color == NULL)
        return false;
    if(kind == CRAZYPOD_CHOICE_MAIN_MENU_ITEM_ACTIONS) {
        enum crazypod_app_id app_id = (enum crazypod_app_id)id;
        *color = !crazypod_apps_is_fixed(app_id) && index == 0
            ? (crazypod_apps_is_enabled(app_id) ? 0xFF4D59 : 0x47E69A)
            : 0x2EBFFF;
        return true;
    }
    return false;
}

static bool action_layout(
    int kind_value, int id, void *context)
{
    enum crazypod_choice_kind kind =
        (enum crazypod_choice_kind)kind_value;

    (void)id;
    (void)context;
    return kind == CRAZYPOD_CHOICE_MAIN_MENU_ITEM_ACTIONS ||
        kind == CRAZYPOD_CHOICE_ROUTE_ACTIONS;
}

static lv_obj_t *create_panel(
    lv_obj_t *parent, int x, int y,
    int width, int height, void *context)
{
    (void)context;
    return crazypod_overlay_glass_panel(
        parent, x, y, width, height);
}

static lv_obj_t *create_underlay(
    lv_obj_t *parent, void *context)
{
    (void)context;
    return crazypod_desktop_native_create_modal_underlay(parent);
}

static void animate_panel(
    lv_obj_t *panel, int target_y, void *context)
{
    (void)context;
    crazypod_popup_animate(panel, target_y);
}

void crazypod_choice_coordinator_configure(
    const struct crazypod_choice_coordinator_host *new_host)
{
    if(new_host != NULL)
        host = *new_host;
}

static void show_overlay(
    enum crazypod_choice_kind kind, int id, int selected)
{
    const struct crazypod_choice_overlay_callbacks callbacks = {
        .count = choice_count,
        .current_index = current_index,
        .title = choice_title,
        .item_title = item_title,
        .item_color = item_color,
        .action_layout = action_layout,
        .create_underlay = create_underlay,
        .create_panel = create_panel,
        .animate_panel = animate_panel,
    };

    if(crazypod_choice_coordinator_visible() ||
       crazypod_now_playing_overlay_visible())
        crazypod_desktop_native_preserve_modal_underlay();
    if(crazypod_now_playing_overlay_visible())
        crazypod_now_playing_overlay_dismiss(false);
    if(crazypod_coverflow_active()) {
        crazypod_coverflow_set_compositing_suspended(true);
        crazypod_overlay_glass_prepare(false);
    }
    else
        crazypod_overlay_glass_prepare(true);
    crazypod_choice_overlay_show(
        host.parent, kind, id, selected,
        host.metadata_font, &callbacks);
}

void crazypod_choice_coordinator_reset(void)
{
    route_action_depth = 0;
    receipt_until = 0;
    receipt_refresh_route = false;
    hold_action = CHOICE_HOLD_NONE;
    hold_until = 0;
    crazypod_choice_overlay_reset();
}

bool crazypod_choice_coordinator_visible(void)
{
    return crazypod_choice_overlay_visible();
}

void crazypod_choice_coordinator_show(
    enum crazypod_choice_kind kind, int id, int selected)
{
    route_action_depth = 0;
    receipt_until = 0;
    receipt_refresh_route = false;
    hold_action = CHOICE_HOLD_NONE;
    hold_until = 0;
    show_overlay(kind, id, selected);
}

void crazypod_choice_coordinator_show_book_reader_actions(void)
{
    /* Old public API; the music edition has no reader modal. */
}

bool crazypod_choice_coordinator_handle_select_repeat(void)
{
    /* No destructive long-press action is exposed by music routes. */
    return false;
}

bool crazypod_choice_coordinator_cancel_select_hold(void)
{
    if(!crazypod_choice_coordinator_handle_select_repeat())
        return false;
    cancel_hold(true);
    return true;
}

void crazypod_choice_coordinator_show_route(
    enum crazypod_route route, int group, int selected)
{
    struct route_state *state;

    cancel_hold(false);
    receipt_until = 0;
    receipt_refresh_route = false;
    if(crazypod_choice_overlay_kind() !=
       CRAZYPOD_CHOICE_ROUTE_ACTIONS)
        route_action_depth = 0;
    if(route_action_depth >= ROUTE_ACTION_DEPTH)
        route_action_depth = ROUTE_ACTION_DEPTH - 1;
    state = &route_actions[route_action_depth++];
    state->route = route;
    state->group = group;
    state->selected = selected;
    show_overlay(
        CRAZYPOD_CHOICE_ROUTE_ACTIONS,
        route_action_depth - 1, selected);
}

void crazypod_choice_coordinator_show_receipt(
    const char *label, bool success, long now,
    bool refresh_route)
{
    if(!crazypod_choice_coordinator_visible()) {
        receipt_until = 0;
        receipt_refresh_route = false;
        crazypod_notification_show(
            success ? CRAZYPOD_NOTIFICATION_SUCCESS
                    : CRAZYPOD_NOTIFICATION_ERROR,
            label);
        return;
    }
    receipt_until = now + RECEIPT_TICKS;
    receipt_refresh_route = refresh_route;
    crazypod_choice_overlay_show_receipt(
        label, success, !crazypod_state_reduce_motion());
}

void crazypod_choice_coordinator_dismiss(bool refresh_route)
{
    route_action_depth = 0;
    receipt_until = 0;
    receipt_refresh_route = false;
    hold_action = CHOICE_HOLD_NONE;
    hold_until = 0;
    crazypod_choice_overlay_dismiss();
    if(refresh_route && host.route_available != NULL &&
       host.route_available())
        host.render(false);
}

bool crazypod_choice_coordinator_back(void)
{
    if(!crazypod_choice_coordinator_visible())
        return false;
    cancel_hold(false);
    if(crazypod_choice_overlay_receipt_visible()) {
        crazypod_choice_coordinator_dismiss(
            receipt_refresh_route);
        return true;
    }
    if(crazypod_choice_overlay_kind() ==
           CRAZYPOD_CHOICE_ROUTE_ACTIONS &&
       route_action_depth > 1) {
        struct route_state *state;

        --route_action_depth;
        state = route_action_current();
        show_overlay(
            CRAZYPOD_CHOICE_ROUTE_ACTIONS,
            route_action_depth - 1,
            state != NULL ? state->selected : 0);
        return true;
    }
    crazypod_choice_coordinator_dismiss(true);
    return true;
}

void crazypod_choice_coordinator_move(int direction)
{
    cancel_hold(true);
    crazypod_choice_overlay_move(direction);
}

void crazypod_choice_coordinator_activate(long now)
{
    enum crazypod_choice_kind kind =
        (enum crazypod_choice_kind)crazypod_choice_overlay_kind();
    int id = crazypod_choice_overlay_id();
    int selected = crazypod_choice_overlay_selected();

    if(kind == CRAZYPOD_CHOICE_NONE ||
       kind == CRAZYPOD_CHOICE_RECEIPT ||
       crazypod_choice_overlay_receipt_visible())
        return;
    if(kind == CRAZYPOD_CHOICE_ROUTE_ACTIONS) {
        struct route_state *state = route_action_current();
        if(state != NULL && host.activate_route != NULL) {
            state->selected = selected;
            host.activate_route(state, now);
        }
        return;
    }
    if(kind == CRAZYPOD_CHOICE_MAIN_MENU_ITEM_ACTIONS) {
        enum crazypod_app_id app_id = (enum crazypod_app_id)id;
        bool fixed = crazypod_apps_is_fixed(app_id);
        if((fixed && selected == 0) || (!fixed && selected == 1)) {
            crazypod_choice_coordinator_dismiss(false);
            if(host.begin_main_menu_reorder != NULL)
                host.begin_main_menu_reorder(app_id);
            return;
        }
        if(!fixed && selected == 0 &&
           crazypod_apps_set_enabled(app_id,
               !crazypod_apps_is_enabled(app_id))) {
            enum crazypod_app_id preferred =
                host.selected_app != NULL
                    ? host.selected_app() : CRAZYPOD_APP_INVALID;
            if(host.main_menu_changed != NULL)
                host.main_menu_changed(preferred, app_id);
            crazypod_choice_coordinator_show_receipt(
                CP_TR("Saved"), true, now, true);
        }
        return;
    }
    if(kind == CRAZYPOD_CHOICE_NOW_PLAYING_THEME) {
        bool applied = crazypod_now_playing_theme_select(selected);
        if(applied && host.appearance_changed != NULL)
            host.appearance_changed();
        crazypod_choice_coordinator_show_receipt(
            applied ? CP_TR("Saved") : CP_TR("Failed"),
            applied, now, applied);
        return;
    }
    if(kind == CRAZYPOD_CHOICE_SETTING) {
        bool applied = crazypod_settings_feature_apply_choice(id, selected);
        crazypod_choice_coordinator_show_receipt(
            applied ? CP_TR("Saved") : CP_TR("Failed"),
            applied, now, applied);
    }
}

const struct route_state *crazypod_choice_coordinator_tick(long now)
{
    bool refresh;
    if(receipt_until == 0 || TIME_BEFORE(now, receipt_until))
        return NULL;
    refresh = receipt_refresh_route;
    crazypod_choice_coordinator_dismiss(refresh);
    return NULL;
}

int crazypod_choice_coordinator_wait_ticks(long now)
{
    long remaining;
    int wait = HZ > 0 ? HZ : 1;

    if(hold_action != CHOICE_HOLD_NONE) {
        remaining = hold_until - now;
        if(remaining <= 0)
            return 1;
        if(remaining < wait)
            wait = (int)remaining;
    }
    if(receipt_until != 0) {
        remaining = receipt_until - now;
        if(remaining <= 0)
            return 1;
        if(remaining < wait)
            wait = (int)remaining;
    }
    return wait;
}

bool crazypod_choice_coordinator_route_should_overlay(
    enum crazypod_route route)
{
    return route == SETTINGS_ROUTE_MAIN_MENU_ACTIONS;
}

bool crazypod_choice_coordinator_confirmation_visible(void)
{
    return false;
}

const struct route_state *
crazypod_choice_coordinator_route_state(void)
{
    return crazypod_choice_overlay_kind() ==
            CRAZYPOD_CHOICE_ROUTE_ACTIONS
        ? route_action_current() : NULL;
}

bool crazypod_choice_coordinator_owns_route_state(
    const struct route_state *state)
{
    int index;

    for(index = 0; index < route_action_depth; ++index) {
        if(state == &route_actions[index])
            return true;
    }
    return false;
}

#endif
