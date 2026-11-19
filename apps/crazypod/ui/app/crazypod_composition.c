#include "config.h"

#ifdef IPOD_6G

#include "../../crazypod_frameclock.h"
#include "../../crazypod_coverflow.h"
#include "../../crazypod_lcd.h"
#include "../../crazypod_state.h"
#include "../features/music/crazypod_music_feature.h"
#include "../features/now_playing/crazypod_now_playing_feature.h"
#include "../shell/crazypod_notification.h"
#include "../navigation/crazypod_render_scheduler.h"
#include "../navigation/crazypod_ui_routes.h"
#include "../presentation/crazypod_alpha_jump_hud.h"
#include "../presentation/crazypod_overlay_glass.h"
#include "../presentation/crazypod_menu_list.h"
#include "../presentation/crazypod_preview_motion.h"
#include "../shell/crazypod_desktop.h"
#include "../shell/crazypod_desktop_native.h"
#include "../shell/crazypod_shell.h"
#include "../shell/crazypod_system_prompts.h"
#include "crazypod_app_input.h"
#include "crazypod_app_launcher.h"
#include "crazypod_choice_coordinator.h"
#include "crazypod_composition.h"
#include "crazypod_menu_preview.h"
#include "crazypod_playback.h"
#include "crazypod_route_actions.h"
#include "crazypod_route_renderer.h"

static struct crazypod_composition_host host;

static void notify_feature_action(
    const char *message, bool success);

static bool route_available(void)
{
    return crazypod_shell_product_active() &&
        crazypod_ui_routes_depth() > 0;
}

static struct route_state *current_route(void)
{
    return crazypod_ui_routes_current();
}

static bool music_route_available(void)
{
    enum crazypod_route route;

    if(!route_available())
        return false;
    route = current_route()->route;
    return route <= MUSIC_ROUTE_NOW_PLAYING ||
        route == PODCASTS_ROUTE_MENU;
}

static void now_overlay_prepare(
    bool refresh, void *context)
{
    bool coverflow_active = crazypod_coverflow_active();

    (void)context;
    if(coverflow_active)
        crazypod_coverflow_set_compositing_suspended(true);
    if(!crazypod_shell_product_active())
        crazypod_desktop_native_prepare_modal();
    crazypod_overlay_glass_prepare(
        refresh && !coverflow_active);
}

static lv_obj_t *now_overlay_panel(
    lv_obj_t *parent, int x, int y,
    int width, int height, void *context)
{
    (void)context;
    return crazypod_overlay_glass_panel(
        parent, x, y, width, height);
}

static void now_overlay_preserve_underlay(void *context)
{
    (void)context;
    crazypod_desktop_native_preserve_modal_underlay();
}

static lv_obj_t *now_overlay_underlay(
    lv_obj_t *parent, void *context)
{
    (void)context;
    return crazypod_desktop_native_create_modal_underlay(parent);
}

static void now_overlay_render(void *context)
{
    (void)context;
    if(route_available() &&
       current_route()->route == MUSIC_ROUTE_NOW_PLAYING)
        host.render(false);
}

static void now_overlay_teardown_complete(void *context)
{
    (void)context;
    if(!crazypod_shell_product_active())
        crazypod_desktop_native_restore_after_modal();
}

static void configure_now_playing(void)
{
    const struct crazypod_now_playing_overlay_host overlay = {
        .parent = crazypod_desktop_screen(),
        .prepare_glass = now_overlay_prepare,
        .preserve_modal_underlay =
            now_overlay_preserve_underlay,
        .create_modal_underlay = now_overlay_underlay,
        .create_panel = now_overlay_panel,
        .notify = notify_feature_action,
        .teardown_complete = now_overlay_teardown_complete,
        .render = now_overlay_render,
    };
    crazypod_now_playing_overlay_configure(&overlay);
}

static void push_now_playing(void)
{
    crazypod_route_actions_push_selected(
        MUSIC_ROUTE_NOW_PLAYING, -1, 0);
}

static void configure_now_navigation(void)
{
    const struct crazypod_now_playing_navigation_host navigation = {
        .push_now_playing = push_now_playing,
        .boost = host.boost,
    };

    crazypod_now_playing_navigation_configure(&navigation);
}

static bool preview_can_render(void)
{
    return route_available() &&
        crazypod_menu_list_matches(current_route()->route);
}

static void preview_render(bool animated)
{
    if(preview_can_render())
        crazypod_menu_preview_render(
            current_route(), animated);
}

static void prepare_loading(void)
{
    crazypod_render_scheduler_reset();
    crazypod_route_renderer_prepare_loading();
}

void crazypod_composition_configure(
    const struct crazypod_composition_host *new_host)
{
    host = *new_host;
    crazypod_shell_create(
        crazypod_desktop_screen(), new_host->boost);
    crazypod_alpha_jump_hud_configure(
        crazypod_shell_product_content());

    const struct crazypod_preview_motion_host preview = {
        .now = new_host->now,
        .reduced_motion = crazypod_state_reduce_motion,
        .can_render = preview_can_render,
        .render = preview_render,
        .boost = new_host->boost,
    };
    const struct crazypod_menu_preview_host menu_preview = {
        .parent = crazypod_shell_product_content(),
        .metadata_font = new_host->metadata_font,
        .item_title = new_host->item_title,
    };
    const struct crazypod_route_renderer_host renderer = {
        .metadata_font = new_host->metadata_font,
        .item_count = new_host->item_count,
        .item_title = new_host->item_title,
        .item_is_current = new_host->item_is_current,
        .render_artwork = new_host->render_artwork,
        .boost = new_host->boost,
    };
    const struct crazypod_render_scheduler_host scheduler = {
        .route_available = route_available,
        .current_route = current_route,
        .render_route = new_host->render,
    };
    const struct crazypod_app_launcher_host launcher = {
        .boost = new_host->set_boost,
        .render = new_host->render,
        .begin_music_scan = new_host->begin_music_scan,
        .request_now_playing =
            crazypod_route_actions_request_now_playing,
        .show_lock = new_host->show_lock,
    };
    const struct crazypod_route_actions_host actions = {
        .render = new_host->render,
        .close_product = new_host->close_product,
        .refresh_menu_rows =
            new_host->refresh_menu_rows,
        .item_count = new_host->item_count,
        .boost = new_host->boost,
        .initial_album_index =
            crazypod_playback_initial_album_index,
    };
    const struct crazypod_playback_host playback = {
        .render = new_host->render,
    };
    const struct crazypod_system_prompts_host prompts = {
        .now = new_host->now,
        .close_product = new_host->close_product,
        .present = crazypod_present_now,
        .cancel_pending_input = crazypod_app_input_cancel_pending,
    };
    const struct crazypod_choice_coordinator_host choices = {
        .parent = crazypod_shell_product_content(),
        .metadata_font = new_host->metadata_font,
        .route_available = route_available,
        .render = new_host->render,
        .item_count = new_host->item_count,
        .item_title = new_host->item_title,
        .item_is_current = new_host->item_is_current,
        .activate_route =
            crazypod_route_actions_activate_overlay_state,
        .selected_app =
            crazypod_route_actions_selected_app,
        .main_menu_changed =
            crazypod_route_actions_main_menu_changed,
        .begin_main_menu_reorder =
            crazypod_route_actions_begin_main_menu_reorder,
    };
    const struct crazypod_music_library_host music = {
        .parent = crazypod_shell_product_content(),
        .prepare_loading_surface = prepare_loading,
        .render_route = new_host->render,
        .route_visible = music_route_available,
    };
    crazypod_preview_motion_configure(&preview);
    crazypod_menu_preview_configure(&menu_preview);
    crazypod_route_renderer_configure(&renderer);
    crazypod_render_scheduler_configure(&scheduler);
    crazypod_app_launcher_configure(&launcher);
    crazypod_route_actions_configure(&actions);
    crazypod_playback_configure(&playback);
    crazypod_system_prompts_configure(&prompts);
    crazypod_choice_coordinator_configure(&choices);
    crazypod_music_library_configure(&music);
    configure_now_playing();
    configure_now_navigation();
}

static void notify_feature_action(
    const char *message, bool success)
{
    crazypod_notification_show(
        success ? CRAZYPOD_NOTIFICATION_SUCCESS
                : CRAZYPOD_NOTIFICATION_ERROR,
        message);
}

#endif
