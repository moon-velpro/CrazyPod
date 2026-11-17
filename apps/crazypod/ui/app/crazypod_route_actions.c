#include "config.h"

#ifdef IPOD_6G

#include "kernel.h"
#include "playlist.h"

#include "../../crazypod_l10n.h"
#include "../../crazypod_apps.h"
#include "../../crazypod_coverflow.h"
#include "../../crazypod_music.h"
#include "../../crazypod_playlist.h"
#include "../../crazypod_state.h"
#include "../features/music/crazypod_music_feature.h"
#include "../features/notes/crazypod_notes_feature.h"
#include "../features/now_playing/crazypod_now_playing_feature.h"
#include "../features/organizer/crazypod_organizer_feature.h"
#include "../features/settings/crazypod_settings_feature.h"
#include "../navigation/crazypod_render_scheduler.h"
#include "../navigation/crazypod_route_registry.h"
#include "../navigation/crazypod_ui_routes.h"
#include "../presentation/crazypod_alpha_jump_hud.h"
#include "../presentation/crazypod_menu_list.h"
#include "../presentation/crazypod_preview_motion.h"
#include "../shell/crazypod_app_catalog.h"
#include "../shell/crazypod_desktop.h"
#include "../shell/crazypod_home_model.h"
#include "../shell/crazypod_notification.h"
#include "crazypod_app_launcher.h"
#include "crazypod_choice_coordinator.h"
#include "crazypod_menu_preview.h"
#include "crazypod_route_actions.h"

#define PREVIEW_SETTLE_TICKS \
    ((HZ * 120 / 1000) > 0 ? (HZ * 120 / 1000) : 1)
#define ALPHA_JUMP_HUD_TICKS \
    ((HZ * 760 / 1000) > 0 ? (HZ * 760 / 1000) : 1)

static struct crazypod_route_actions_host host;
static bool overlay_dispatch;
static bool overlay_changed;
static bool overlay_transition;
static enum crazypod_app_id reorder_preferred =
    CRAZYPOD_APP_INVALID;

static struct route_state *current_route(void)
{
    return crazypod_ui_routes_current();
}

void crazypod_route_actions_configure(
    const struct crazypod_route_actions_host *new_host)
{
    if(new_host != NULL)
        host = *new_host;
}

static void transaction_render(bool transition)
{
    if(overlay_dispatch) {
        overlay_changed = true;
        overlay_transition = overlay_transition || transition;
        return;
    }
    host.render(transition);
}

static bool push_route_state(
    enum crazypod_route route, int group, int selected)
{
    if(!crazypod_route_registry_product_allowed(route) ||
       !crazypod_ui_routes_push(route, group, selected))
        return false;
    crazypod_menu_preview_prefetch(current_route());
    return true;
}

void crazypod_route_actions_push_selected(
    enum crazypod_route route, int group, int selected)
{
    if(!crazypod_route_registry_product_allowed(route))
        return; /* A stale route must not start an abandoned app. */
    if(route == SETTINGS_ROUTE_MAIN_MENU_ACTIONS) {
        crazypod_choice_coordinator_show(
            CRAZYPOD_CHOICE_MAIN_MENU_ITEM_ACTIONS,
            group, selected);
        return;
    }
    if(route == DIY_ROUTE_NOW_PLAYING_THEMES) {
        crazypod_choice_coordinator_show(
            CRAZYPOD_CHOICE_NOW_PLAYING_THEME,
            0, selected);
        return;
    }
    if(crazypod_choice_coordinator_route_should_overlay(route)) {
        crazypod_choice_coordinator_show_route(
            route, group, selected);
        return;
    }
    if(!push_route_state(route, group, selected))
        return;
    transaction_render(true);
}

void crazypod_route_actions_push(
    enum crazypod_route route, int group)
{
    crazypod_route_actions_push_selected(
        route, group, 0);
}

void crazypod_route_actions_request_now_playing(void)
{
    crazypod_now_playing_request_open();
}

bool crazypod_route_actions_confirm_photos(
    const struct route_state *state, long now)
{
    /* Removed photo feature: no user file operation can run from here. */
    (void)state;
    (void)now;
    return false;
}

void crazypod_route_actions_pop(void)
{
    if(crazypod_ui_routes_depth() > 0 &&
       current_route()->route == MUSIC_ROUTE_NOW_PLAYING)
        crazypod_now_playing_theme_close();
    if(crazypod_ui_routes_depth() > 1) {
        crazypod_ui_routes_pop();
        transaction_render(true);
    }
    else
        host.close_product();
}

static void play_selected_track(struct route_state *state)
{
    bool started = false;

    switch(state->route) {
    case MUSIC_ROUTE_ALL:
    case MUSIC_ROUTE_SONGS:
        started = crazypod_music_play(
            CRAZYPOD_SCOPE_ALL, 0, state->selected);
        break;
    case MUSIC_ROUTE_PLAYLIST_SONGS:
        started = crazypod_music_play(
            CRAZYPOD_SCOPE_PLAYLIST,
            state->group, state->selected);
        break;
    case MUSIC_ROUTE_ARTIST_SONGS:
        started = crazypod_music_play(
            CRAZYPOD_SCOPE_ARTIST,
            state->group, state->selected);
        break;
    case MUSIC_ROUTE_ALBUM_SONGS:
        started = crazypod_music_play(
            CRAZYPOD_SCOPE_ALBUM,
            state->group, state->selected);
        break;
    case MUSIC_ROUTE_QUEUE:
        if(state->selected >= 0 &&
           state->selected < crazypod_queue_count()) {
            playlist_start(state->selected, 0, 0);
            started = true;
        }
        break;
    case MUSIC_ROUTE_SEARCH_RESULTS:
        started = crazypod_music_play_search(
            crazypod_music_search_query(), state->selected);
        break;
    default:
        break;
    }
    if(started) {
        crazypod_state_forget_resume();
        crazypod_state_mark_dirty();
        crazypod_route_actions_request_now_playing();
    }
    else {
        crazypod_notification_show(
            CRAZYPOD_NOTIFICATION_ERROR,
            CP_TR("No track available"));
    }
}

void crazypod_route_actions_begin_note(
    uint32_t id, bool resume_draft)
{
    crazypod_notes_feature_begin_editor(id, resume_draft);
    crazypod_route_actions_push_selected(
        NOTES_ROUTE_COMPOSER, -1, 0);
}

bool crazypod_route_actions_note_dirty(void)
{
    return crazypod_notes_feature_editor_dirty();
}

void crazypod_route_actions_service_notes(void)
{
    crazypod_notes_feature_service_editor();
}

void crazypod_route_actions_show_calendar_day(int date)
{
    int root = -1;
    int i;

    crazypod_organizer_feature_set_focus_date(date);
    for(i = crazypod_ui_routes_depth() - 1; i >= 0; --i) {
        if(crazypod_ui_routes_at(i)->route ==
           CALENDAR_ROUTE_MENU) {
            root = i;
            break;
        }
    }
    if(root < 0) {
        root = 0;
        crazypod_ui_routes_reset(
            CALENDAR_ROUTE_MENU, -1, 0);
    }
    crazypod_ui_routes_truncate(root + 1);
    crazypod_ui_routes_push(
        CALENDAR_ROUTE_DAY_EVENTS, -1, 0);
}

/* Legacy reorder state is still loadable, but it no longer controls the
 * fixed four-destination Home. Persist app order without changing its focus. */
static void persist_main_menu(
    enum crazypod_app_id preferred)
{
    (void)preferred;
    crazypod_state_mark_dirty();
    crazypod_state_save(true);
}

enum crazypod_app_id crazypod_route_actions_selected_app(void)
{
    switch(crazypod_desktop_selected()) {
    case CRAZYPOD_HOME_NOW_PLAYING:
    case CRAZYPOD_HOME_MUSIC:
        return CRAZYPOD_APP_MUSIC;
    case CRAZYPOD_HOME_PODCASTS:
        return CRAZYPOD_APP_PODCASTS;
    case CRAZYPOD_HOME_SETTINGS:
        return CRAZYPOD_APP_SETTINGS;
    default:
        return CRAZYPOD_APP_INVALID;
    }
}

void crazypod_route_actions_main_menu_changed(
    enum crazypod_app_id preferred,
    enum crazypod_app_id changed)
{
    struct route_state *parent = current_route();

    if(parent != NULL && parent->route ==
       SETTINGS_ROUTE_MAIN_MENU)
        parent->selected =
            crazypod_apps_order_index(changed);
    persist_main_menu(preferred);
}

void crazypod_route_actions_begin_main_menu_reorder(
    enum crazypod_app_id id)
{
    struct route_state *state = current_route();

    if(state == NULL ||
       state->route != SETTINGS_ROUTE_MAIN_MENU ||
       !crazypod_apps_is_known(id))
        return;
    reorder_preferred =
        crazypod_route_actions_selected_app();
    crazypod_settings_feature_begin_main_menu_reorder(id);
    state->selected = crazypod_apps_order_index(id);
    host.render(false);
}

void crazypod_route_actions_commit_pending_main_menu_reorder(void)
{
    enum crazypod_app_id id;

    if(!crazypod_settings_feature_main_menu_reordering())
        return;
    id = crazypod_settings_feature_finish_main_menu_reorder();
    if(crazypod_apps_is_known(id))
        persist_main_menu(reorder_preferred);
    reorder_preferred = CRAZYPOD_APP_INVALID;
    if(crazypod_apps_is_known(id))
        crazypod_choice_coordinator_show_receipt(
            CP_TR("Saved"), true, current_tick, false);
}

static void show_main_menu_actions(enum crazypod_app_id id)
{
    crazypod_choice_coordinator_show(
        CRAZYPOD_CHOICE_MAIN_MENU_ITEM_ACTIONS,
        id, 0);
}

static void show_setting_choices(int item, int selected)
{
    crazypod_choice_coordinator_show(
        CRAZYPOD_CHOICE_SETTING, item, selected);
}

/* Reuse the already validated async Shuffle app path.  A Music menu
 * action does not reopen the retired 17-app Home or add a new route. */
static void shuffle_all_from_music(void)
{
    if(crazypod_music_library_loaded() &&
       crazypod_music_track_count() == 0) {
        crazypod_notification_show(
            CRAZYPOD_NOTIFICATION_ERROR,
            CP_TR("No track available"));
        return;
    }
    crazypod_app_launcher_open(CRAZYPOD_APP_SHUFFLE);
}

/* Reuse the already validated async Shuffle app path.  A Music menu
 * action does not reopen the retired 17-app Home or add a new route. */
static void shuffle_all_from_music(void)
{
    if(crazypod_music_library_loaded() &&
       crazypod_music_track_count() == 0) {
        crazypod_notification_show(
            CRAZYPOD_NOTIFICATION_ERROR,
            CP_TR("No track available"));
        return;
    }
    crazypod_app_launcher_open(CRAZYPOD_APP_SHUFFLE);
}

static bool activate_music(
    struct route_state *state, long now)
{
    const struct crazypod_feature *feature =
        crazypod_route_registry_feature(state->route);

    if(feature == NULL)
        return false;
    (void)now;
    if(feature->id == CRAZYPOD_FEATURE_MUSIC ||
       feature->id == CRAZYPOD_FEATURE_NOW_PLAYING) {
        const struct crazypod_music_activation_host actions = {
            .render = transaction_render,
            .push = crazypod_route_actions_push,
            .push_selected =
                crazypod_route_actions_push_selected,
            .initial_album_index =
                host.initial_album_index,
            .request_now_playing =
                crazypod_route_actions_request_now_playing,
            .show_now_actions =
                crazypod_now_playing_overlay_show_actions,
            .shuffle_all = shuffle_all_from_music,
        };

        return crazypod_music_feature_activate(
            state, &actions);
    }
    return false;
}

static bool activate_domain(
    struct route_state *state, long now)
{
    const struct crazypod_feature *feature =
        crazypod_route_registry_feature(state->route);

    if(feature == NULL)
        return false;
    if(feature->id == CRAZYPOD_FEATURE_MUSIC ||
       feature->id == CRAZYPOD_FEATURE_NOW_PLAYING)
        return activate_music(state, now);
    if(feature->id == CRAZYPOD_FEATURE_SETTINGS) {
        const struct crazypod_settings_activation_host actions = {
            .selected_app =
                crazypod_route_actions_selected_app,
            .push = crazypod_route_actions_push,
            .render = transaction_render,
            .main_menu_changed =
                crazypod_route_actions_main_menu_changed,
            .show_main_menu_actions =
                show_main_menu_actions,
            .show_choices = show_setting_choices,
        };

        return crazypod_settings_feature_activate(state, &actions);
    }
    return false;
}

void crazypod_route_actions_activate(long now)
{
    struct route_state *state = current_route();
    if(state == NULL)
        return;
    if(!crazypod_route_registry_product_allowed(state->route)) {
        host.close_product();
        return;
    }
    if(state->route == SETTINGS_ROUTE_MAIN_MENU &&
       crazypod_settings_feature_main_menu_reordering()) {
        enum crazypod_app_id id =
            crazypod_settings_feature_finish_main_menu_reorder();

        if(crazypod_apps_is_known(id)) {
            state->selected = crazypod_apps_order_index(id);
            persist_main_menu(reorder_preferred);
            reorder_preferred = CRAZYPOD_APP_INVALID;
            host.render(false);
            crazypod_choice_coordinator_show_receipt(
                CP_TR("Saved"), true, now, false);
        }
        return;
    }
    if(state->route == MUSIC_ROUTE_NOW_PLAYING &&
       crazypod_now_playing_overlay_visible()) {
        crazypod_now_playing_overlay_activate();
        return;
    }
    if(crazypod_choice_coordinator_visible()) {
        crazypod_choice_coordinator_activate(now);
        return;
    }
    if(activate_domain(state, now))
        return;
    if(crazypod_route_registry_is_shell(state->route)) {
        const struct crazypod_app_descriptor *app =
            crazypod_app_catalog_find(
                crazypod_apps_hidden_id(state->selected));

        if(app != NULL)
            crazypod_app_launcher_open(app->id);
        return;
    }
    play_selected_track(state);
}

void crazypod_route_actions_activate_overlay_state(
    const struct route_state *source, long now)
{
    struct route_state state;
    if(source == NULL ||
       !crazypod_route_registry_product_allowed(source->route))
        return;
    state = *source;
    overlay_dispatch = true;
    overlay_changed = false;
    overlay_transition = false;
    (void)activate_domain(&state, now);
    overlay_dispatch = false;

    if(overlay_transition) {
        crazypod_choice_coordinator_dismiss(false);
        host.render(true);
    }
    else if(overlay_changed) {
        crazypod_choice_coordinator_show_receipt(
            CP_TR("Saved"), true, now, true);
    }
}

void crazypod_route_actions_move(int direction, long now)
{
    struct route_state *state = current_route();
    int count;
    int next;

    if(state == NULL)
        return;
    if(state->route == SETTINGS_ROUTE_MAIN_MENU &&
       crazypod_settings_feature_main_menu_reordering()) {
        int step = direction < 0 ? -1 : 1;
        int remaining = direction < 0 ? -direction : direction;
        enum crazypod_app_id id =
            crazypod_settings_feature_main_menu_reorder_id();
        bool changed = false;

        while(remaining-- > 0)
            changed =
                crazypod_settings_feature_move_main_menu_item(step) ||
                changed;
        if(changed) {
            state->selected = crazypod_apps_order_index(id);
            host.render(false);
        }
        return;
    }
    count = host.item_count(state);
    if(crazypod_choice_coordinator_visible()) {
        crazypod_choice_coordinator_move(direction);
        return;
    }
    if(state->route == MUSIC_ROUTE_NOW_PLAYING) {
        if(crazypod_now_playing_overlay_visible())
            crazypod_now_playing_overlay_move(direction);
        else
            crazypod_now_playing_adjust_volume(direction);
        return;
    }
    if(count <= 0)
        return;
    host.boost(HZ / 3);
    if(state->route == MUSIC_ROUTE_ALBUM_FLOW) {
        next = crazypod_coverflow_step(direction);
        if(next != state->selected)
            state->selected = next;
        return;
    }
    next = state->selected + direction;
    if(next < 0)
        next = 0;
    if(next >= count)
        next = count - 1;
    if(next == state->selected)
        return;
    state->selected = next;
    crazypod_menu_preview_prefetch(state);
    if(crazypod_menu_preview_is_skeuomorphic_route(
           state->route))
        crazypod_preview_motion_set_direction(direction);
    if(state->route == MUSIC_ROUTE_SEARCH) {
        host.refresh_menu_rows(state);
        return;
    }
    if(crazypod_menu_list_matches(state->route)) {
        host.refresh_menu_rows(state);
        crazypod_render_scheduler_schedule_preview(
            now + PREVIEW_SETTLE_TICKS);
    }
    else
        crazypod_render_scheduler_schedule_route(now);
}

bool crazypod_route_actions_alpha_jump(
    int direction, long now)
{
    struct route_state *state = current_route();
    int target;
    char key;

    if(state == NULL ||
       !crazypod_music_feature_alpha_jump_target(
           state, direction, &target, &key) ||
       target == state->selected)
        return false;
    host.boost(HZ / 3);
    state->selected = target;
    if(crazypod_menu_preview_is_music_route(state->route))
        crazypod_menu_preview_prefetch(state);
    if(crazypod_menu_preview_is_skeuomorphic_route(
           state->route))
        crazypod_preview_motion_set_direction(direction);
    if(crazypod_menu_list_matches(state->route)) {
        host.refresh_menu_rows(state);
        crazypod_render_scheduler_schedule_preview(
            now + PREVIEW_SETTLE_TICKS);
    }
    else
        crazypod_render_scheduler_schedule_route(now);
    crazypod_alpha_jump_hud_show(
        key, now, ALPHA_JUMP_HUD_TICKS);
    return true;
}

#endif
