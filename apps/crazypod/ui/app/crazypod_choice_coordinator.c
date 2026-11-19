#include "config.h"

#include "../../crazypod_l10n.h"

#ifdef IPOD_6G

#include "kernel.h"

#include "../../crazypod_coverflow.h"
#include "../../crazypod_state.h"
#include "../features/settings/crazypod_settings_feature.h"
#include "../navigation/crazypod_route_query.h"
#include "../presentation/crazypod_choice_overlay.h"
#include "../presentation/crazypod_overlay_glass.h"
#include "../presentation/crazypod_popup_motion.h"
#include "../shell/crazypod_app_catalog.h"
#include "../shell/crazypod_desktop_native.h"
#include "../shell/crazypod_notification.h"
#include "crazypod_choice_coordinator.h"

#define RECEIPT_TICKS \
    ((HZ * 800 / 1000) > 0 ? (HZ * 800 / 1000) : 1)

static struct crazypod_choice_coordinator_host host;
static struct route_state action_route;
static bool action_route_active;
static long receipt_until;
static bool receipt_refresh_route;

static const struct route_state *current_action_route(void)
{
    return action_route_active ? &action_route : NULL;
}

static const struct crazypod_app_descriptor *main_menu_app(int id)
{
    return crazypod_app_catalog_find((enum crazypod_app_id)id);
}

static int choice_count(int kind_value, int id, void *context)
{
    enum crazypod_choice_kind kind =
        (enum crazypod_choice_kind)kind_value;
    const struct route_state *state = current_action_route();

    (void)context;
    switch(kind) {
    case CRAZYPOD_CHOICE_SETTING:
        return crazypod_settings_feature_choice_count(id);
    case CRAZYPOD_CHOICE_MAIN_MENU_ITEM_ACTIONS:
        return crazypod_apps_is_fixed(
            (enum crazypod_app_id)id) ? 1 : 2;
    case CRAZYPOD_CHOICE_ROUTE_ACTIONS:
        return state != NULL && host.item_count != NULL
            ? host.item_count(state) : 0;
    case CRAZYPOD_CHOICE_RECEIPT:
        return 1;
    default:
        return 0;
    }
}

static int current_index(int kind_value, int id, void *context)
{
    enum crazypod_choice_kind kind =
        (enum crazypod_choice_kind)kind_value;
    const struct route_state *state = current_action_route();

    (void)context;
    switch(kind) {
    case CRAZYPOD_CHOICE_SETTING:
        return crazypod_settings_feature_choice_index(id);
    case CRAZYPOD_CHOICE_ROUTE_ACTIONS:
        return state != NULL && host.item_is_current != NULL &&
            host.item_is_current(state, state->selected)
                ? state->selected : -1;
    default:
        return -1;
    }
}

static const char *choice_title(
    int kind_value, int id, void *context)
{
    enum crazypod_choice_kind kind =
        (enum crazypod_choice_kind)kind_value;
    const struct route_state *state = current_action_route();

    (void)context;
    switch(kind) {
    case CRAZYPOD_CHOICE_SETTING:
        return crazypod_settings_feature_choice_item_title(id);
    case CRAZYPOD_CHOICE_MAIN_MENU_ITEM_ACTIONS: {
        const struct crazypod_app_descriptor *app =
            main_menu_app(id);

        return app != NULL ? app->name : CP_TR("MAIN MENU");
    }
    case CRAZYPOD_CHOICE_ROUTE_ACTIONS:
        return state != NULL
            ? crazypod_route_query_title(state) : "";
    default:
        return "";
    }
}

static const char *item_title(
    int kind_value, int id, int index, void *context)
{
    enum crazypod_choice_kind kind =
        (enum crazypod_choice_kind)kind_value;
    const struct route_state *state = current_action_route();

    (void)context;
    switch(kind) {
    case CRAZYPOD_CHOICE_SETTING:
        return crazypod_settings_feature_choice_title(id, index);
    case CRAZYPOD_CHOICE_MAIN_MENU_ITEM_ACTIONS: {
        enum crazypod_app_id app_id =
            (enum crazypod_app_id)id;

        if(crazypod_apps_is_fixed(app_id))
            return index == 0 ? CP_TR("Move") : "";
        if(index == 0)
            return crazypod_apps_is_enabled(app_id)
                ? CP_TR("Hide") : CP_TR("Show");
        return index == 1 ? CP_TR("Move") : "";
    }
    case CRAZYPOD_CHOICE_ROUTE_ACTIONS:
        return state != NULL && host.item_title != NULL
            ? host.item_title(state, index) : "";
    default:
        return "";
    }
}

static bool item_color(
    int kind_value, int id, int index,
    uint32_t *color, void *context)
{
    enum crazypod_choice_kind kind =
        (enum crazypod_choice_kind)kind_value;

    (void)context;
    if(color == NULL ||
       kind != CRAZYPOD_CHOICE_MAIN_MENU_ITEM_ACTIONS)
        return false;
    if(!crazypod_apps_is_fixed((enum crazypod_app_id)id) &&
       index == 0)
        *color = crazypod_apps_is_enabled(
            (enum crazypod_app_id)id) ? 0xFF4D59 : 0x47E69A;
    else
        *color = 0x2EBFFF;
    return true;
}

static bool action_layout(
    int kind_value, int id, void *context)
{
    (void)id;
    (void)context;
    return kind_value == CRAZYPOD_CHOICE_MAIN_MENU_ITEM_ACTIONS ||
        kind_value == CRAZYPOD_CHOICE_ROUTE_ACTIONS;
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

    if(crazypod_now_playing_overlay_visible())
        crazypod_now_playing_overlay_dismiss(false);
    if(crazypod_coverflow_active())
        crazypod_coverflow_set_compositing_suspended(true);
    crazypod_choice_overlay_show(
        host.parent, kind, id, selected,
        host.metadata_font, &callbacks);
}

void crazypod_choice_coordinator_reset(void)
{
    action_route_active = false;
    receipt_until = 0;
    receipt_refresh_route = false;
    crazypod_choice_overlay_reset();
}

bool crazypod_choice_coordinator_visible(void)
{
    return crazypod_choice_overlay_visible();
}

void crazypod_choice_coordinator_show(
    enum crazypod_choice_kind kind, int id, int selected)
{
    if(kind != CRAZYPOD_CHOICE_SETTING &&
       kind != CRAZYPOD_CHOICE_MAIN_MENU_ITEM_ACTIONS)
        return;
    action_route_active = false;
    receipt_until = 0;
    receipt_refresh_route = false;
    show_overlay(kind, id, selected);
}

void crazypod_choice_coordinator_show_route(
    enum crazypod_route route, int group, int selected)
{
    if(!crazypod_choice_coordinator_route_should_overlay(route))
        return;
    receipt_until = 0;
    receipt_refresh_route = false;
    action_route.route = route;
    action_route.group = group;
    action_route.selected = selected;
    action_route_active = true;
    show_overlay(CRAZYPOD_CHOICE_ROUTE_ACTIONS, 0, selected);
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
    action_route_active = false;
    receipt_until = 0;
    receipt_refresh_route = false;
    crazypod_choice_overlay_dismiss();
    if(refresh_route && host.route_available != NULL &&
       host.route_available())
        host.render(false);
}

bool crazypod_choice_coordinator_back(void)
{
    if(!crazypod_choice_coordinator_visible())
        return false;
    crazypod_choice_coordinator_dismiss(
        crazypod_choice_overlay_receipt_visible()
            ? receipt_refresh_route : true);
    return true;
}

void crazypod_choice_coordinator_move(int direction)
{
    crazypod_choice_overlay_move(direction);
}

void crazypod_choice_coordinator_activate(long now)
{
    enum crazypod_choice_kind kind =
        (enum crazypod_choice_kind)
            crazypod_choice_overlay_kind();
    int id = crazypod_choice_overlay_id();
    int selected = crazypod_choice_overlay_selected();

    if(kind == CRAZYPOD_CHOICE_NONE ||
       kind == CRAZYPOD_CHOICE_RECEIPT ||
       crazypod_choice_overlay_receipt_visible())
        return;
    if(kind == CRAZYPOD_CHOICE_ROUTE_ACTIONS) {
        if(action_route_active && host.activate_route != NULL) {
            action_route.selected = selected;
            host.activate_route(&action_route, now);
        }
        return;
    }
    if(kind == CRAZYPOD_CHOICE_MAIN_MENU_ITEM_ACTIONS) {
        enum crazypod_app_id app_id = (enum crazypod_app_id)id;
        bool fixed = crazypod_apps_is_fixed(app_id);

        if((fixed && selected == 0) ||
           (!fixed && selected == 1)) {
            crazypod_choice_coordinator_dismiss(false);
            if(host.begin_main_menu_reorder != NULL)
                host.begin_main_menu_reorder(app_id);
            return;
        }
        if(!fixed && selected == 0 &&
           crazypod_apps_set_enabled(
               app_id, !crazypod_apps_is_enabled(app_id))) {
            enum crazypod_app_id preferred =
                host.selected_app != NULL
                    ? host.selected_app()
                    : CRAZYPOD_APP_INVALID;

            if(host.main_menu_changed != NULL)
                host.main_menu_changed(preferred, app_id);
            crazypod_choice_coordinator_show_receipt(
                CP_TR("Saved"), true, now, true);
        }
        return;
    }
    if(kind == CRAZYPOD_CHOICE_SETTING) {
        bool applied =
            crazypod_settings_feature_apply_choice(id, selected);

        crazypod_choice_coordinator_show_receipt(
            applied ? CP_TR("Saved") : CP_TR("Failed"),
            applied, now, applied);
    }
}

void crazypod_choice_coordinator_tick(long now)
{
    bool refresh;

    if(receipt_until == 0 || TIME_BEFORE(now, receipt_until))
        return;
    refresh = receipt_refresh_route;
    crazypod_choice_coordinator_dismiss(refresh);
}

int crazypod_choice_coordinator_wait_ticks(long now)
{
    long remaining;
    int wait = HZ > 0 ? HZ : 1;

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

#endif
