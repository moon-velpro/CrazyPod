#include "config.h"

#if defined(IPOD_6G) && defined(SIMULATOR)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "button.h"
#include "kernel.h"
#include "lvgl.h"

#include "../../crazypod_l10n.h"
#include "../../crazypod_music.h"
#include "../../crazypod_runtime_font.h"
#include "../../crazypod_state.h"
#include "../features/now_playing/crazypod_now_playing_feature.h"
#include "../navigation/crazypod_route_query.h"
#include "../shell/crazypod_desktop.h"
#include "../shell/crazypod_headphone_popup.h"
#include "../shell/crazypod_lock_screen.h"
#include "../shell/crazypod_notification.h"
#include "../shell/crazypod_shell.h"
#include "crazypod_app_input.h"
#include "crazypod_simulator_snapshot.h"

/* Simulator scenarios are a testing facade for the shipping music player.
 * Retired apps and CPK themes do not have screenshot entry points. */

long crazypod_simulator_snapshot_settle_ticks(void)
{
    const char *value =
        getenv("CRAZYPOD_SIM_DUMP_SETTLE_MS");
    char *end = NULL;
    long milliseconds = value != NULL
        ? strtol(value, &end, 10) : 500;

    if(end == value || (end != NULL && *end != '\0') ||
       milliseconds < 50 || milliseconds > 10000)
        milliseconds = 500;
    return MAX(1, milliseconds * HZ / 1000);
}

static struct route_state *current_route(void)
{
    return crazypod_ui_routes_current();
}

static int item_count(void)
{
    return crazypod_route_query_item_count(
        current_route(), crazypod_music_search_query());
}

static void add_runtime_font_probe_label(
    lv_obj_t *parent, const char *text, const lv_font_t *font,
    int x, int y, int width, int height, uint32_t color)
{
    lv_obj_t *label = lv_label_create(parent);

    lv_label_set_text(label, text);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_size(label, width, height);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, 0);
}

static bool open_runtime_font_catalog(void)
{
    static const struct {
        const char *name;
        enum crazypod_font_family family;
        unsigned weight;
    } probes[] = {
        { "SANS 100", CRAZYPOD_FONT_FAMILY_SYSTEM, 100 },
        { "SANS 200", CRAZYPOD_FONT_FAMILY_SYSTEM, 200 },
        { "SANS 300", CRAZYPOD_FONT_FAMILY_SYSTEM, 300 },
        { "SANS 400", CRAZYPOD_FONT_FAMILY_SYSTEM, 400 },
        { "SANS 500", CRAZYPOD_FONT_FAMILY_SYSTEM, 500 },
        { "SANS 600", CRAZYPOD_FONT_FAMILY_SYSTEM, 600 },
        { "SANS 700", CRAZYPOD_FONT_FAMILY_SYSTEM, 700 },
        { "SANS 800", CRAZYPOD_FONT_FAMILY_SYSTEM, 800 },
        { "SANS 900", CRAZYPOD_FONT_FAMILY_SYSTEM, 900 },
        { "SERIF 100", CRAZYPOD_FONT_FAMILY_SERIF, 100 },
        { "SERIF 200", CRAZYPOD_FONT_FAMILY_SERIF, 200 },
        { "SERIF 300", CRAZYPOD_FONT_FAMILY_SERIF, 300 },
        { "SERIF 400", CRAZYPOD_FONT_FAMILY_SERIF, 400 },
        { "SERIF 500", CRAZYPOD_FONT_FAMILY_SERIF, 500 },
        { "SERIF 600", CRAZYPOD_FONT_FAMILY_SERIF, 600 },
        { "SERIF 700", CRAZYPOD_FONT_FAMILY_SERIF, 700 },
        { "SERIF 800", CRAZYPOD_FONT_FAMILY_SERIF, 800 },
        { "SERIF 900", CRAZYPOD_FONT_FAMILY_SERIF, 900 },
        { "MONO 100", CRAZYPOD_FONT_FAMILY_MONO, 100 },
        { "MONO 200", CRAZYPOD_FONT_FAMILY_MONO, 200 },
        { "MONO 300", CRAZYPOD_FONT_FAMILY_MONO, 300 },
        { "MONO 400", CRAZYPOD_FONT_FAMILY_MONO, 400 },
        { "MONO 500", CRAZYPOD_FONT_FAMILY_MONO, 500 },
        { "MONO 600", CRAZYPOD_FONT_FAMILY_MONO, 600 },
        { "MONO 700", CRAZYPOD_FONT_FAMILY_MONO, 700 },
        { "MONO 800", CRAZYPOD_FONT_FAMILY_MONO, 800 },
        { "MONO 900", CRAZYPOD_FONT_FAMILY_MONO, 900 },
    };
    static const uint32_t colors[] = {
        0x5ac8fa, 0xf5f5f7, 0xffcc00,
        0x34c759, 0xff375f, 0xbf5af2,
    };
    lv_obj_t *catalog = lv_obj_create(NULL);
    unsigned index;

    if(!crazypod_runtime_fonts_ready())
        return false;
    crazypod_shell_open_product();
    lv_obj_add_flag(lv_layer_top(), LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(lv_layer_sys(), LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_style_all(catalog);
    lv_obj_set_pos(catalog, 0, 0);
    lv_obj_set_size(catalog, 320, 240);
    lv_obj_set_style_bg_color(catalog, lv_color_hex(0x101114), 0);
    lv_obj_set_style_bg_opa(catalog, LV_OPA_COVER, 0);
    lv_screen_load(catalog);
    add_runtime_font_probe_label(
        catalog, "一个人的精彩\n萧亚轩 / 红蔷薇",
        crazypod_runtime_font(), 10, 8, 300, 42, 0xf5f5f7);
    for(index = 0; index < ARRAYLEN(probes); ++index) {
        int column = index / 9;
        int row = index % 9;

        add_runtime_font_probe_label(
            catalog, probes[index].name,
            crazypod_runtime_font_resolve(
                probes[index].family, 12, probes[index].weight,
                CRAZYPOD_FONT_STYLE_NORMAL, 16),
            8 + column * 104, 50 + row * 20, 98, 18,
            colors[index % ARRAYLEN(colors)]);
    }
    return true;
}

static void select_bounded(
    const struct crazypod_simulator_snapshot_host *host,
    int requested)
{
    int count = item_count();

    if(requested < 0)
        requested = 0;
    if(requested >= count)
        requested = count > 0 ? count - 1 : 0;
    current_route()->selected = requested;
    host->render(false);
}

static bool open_lock_playback_snapshot(
    const struct crazypod_simulator_snapshot_host *host,
    bool paused)
{
    if(host->show_lock == NULL ||
       crazypod_music_track_count() <= 0 ||
       !crazypod_music_play(CRAZYPOD_SCOPE_ALL, 0, 0))
        return false;
    if(paused)
        audio_pause();
    host->show_lock(false);
    return true;
}

static bool open_lock_cjk_snapshot(
    const struct crazypod_simulator_snapshot_host *host)
{
    const struct crazypod_lock_media_snapshot snapshot = {
        .active = true,
        .playing = true,
        .metadata_ready = true,
        .track_path = "/Music/crazypod-lock-cjk-test.mp3",
        .title = "夜航测试歌曲",
        .artist = "疯狂播客乐队",
        .album = "嵌入封面专辑",
        .elapsed_ms = 42000,
        .length_ms = 198000,
    };

    if(host->show_lock == NULL)
        return false;
    audio_stop();
    host->show_lock(false);
    crazypod_lock_screen_update_media(&snapshot);
    return true;
}

static bool open_now_playing_default_actions(
    const struct crazypod_simulator_snapshot_host *host)
{
    if(!open_now_playing_default_media(host))
        return false;
    crazypod_app_input_handle(BUTTON_SELECT, 0, current_tick);
    return crazypod_now_playing_overlay_kind() ==
        CRAZYPOD_NOW_OVERLAY_ACTIONS;
}

static bool open_now_playing_default_media(
    const struct crazypod_simulator_snapshot_host *host)
{
    if(crazypod_music_track_count() < 2 ||
       !crazypod_music_play(CRAZYPOD_SCOPE_ALL, 0, 0))
        return false;
    host->open_root_route(MUSIC_ROUTE_NOW_PLAYING);
    return true;
}

static bool open_now_playing_default_mode(
    const struct crazypod_simulator_snapshot_host *host)
{
    if(!open_now_playing_default_media(host))
        return false;
    crazypod_now_playing_overlay_show_actions();
    crazypod_now_playing_overlay_move(2);
    crazypod_now_playing_overlay_activate();
    return crazypod_now_playing_overlay_kind() ==
        CRAZYPOD_NOW_OVERLAY_PLAYBACK;
}

static bool open_now_playing_default_progress(
    const struct crazypod_simulator_snapshot_host *host)
{
    if(!open_now_playing_default_media(host))
        return false;
    crazypod_now_playing_overlay_show_actions();
    crazypod_now_playing_overlay_move(4);
    crazypod_now_playing_overlay_activate();
    return crazypod_now_playing_overlay_kind() ==
        CRAZYPOD_NOW_OVERLAY_PROGRESS;
}

static bool open_now_playing_default_lyrics(
    const struct crazypod_simulator_snapshot_host *host)
{
    if(!open_now_playing_default_media(host))
        return false;
    crazypod_now_playing_overlay_show_actions();
    crazypod_now_playing_overlay_move(3);
    crazypod_now_playing_overlay_activate();
    crazypod_now_playing_overlay_dismiss(true);
    host->render(false);
    return crazypod_now_playing_lyrics_mode();
}

bool crazypod_simulator_snapshot_prepare(
    const struct crazypod_simulator_snapshot_host *host)
{
    const char *screen = getenv("CRAZYPOD_SIM_SCREEN");
    const char *language = getenv("CRAZYPOD_SIM_LANGUAGE");
    int preview_index;
    int language_index;

    if(host == NULL || getenv("CRAZYPOD_SIM_DUMP") == NULL)
        return false;
    if(language != NULL) {
        for(language_index = 0;
            language_index < CRAZYPOD_LANGUAGE_COUNT;
            ++language_index) {
            if(strcmp(language,
                      crazypod_language_code(
                          (enum crazypod_language)language_index)) == 0) {
                crazypod_language_set(
                    (enum crazypod_language)language_index);
                break;
            }
        }
    }
    if(screen == NULL || strcmp(screen, "home") == 0)
        return true;
    if(strcmp(screen, "notification-success") == 0) {
        crazypod_notification_show_for(
            CRAZYPOD_NOTIFICATION_SUCCESS, CP_TR("Saved"), 5000);
        return true;
    }
    if(strcmp(screen, "notification-error") == 0) {
        crazypod_notification_show_for(
            CRAZYPOD_NOTIFICATION_ERROR,
            CP_TR("Screenshot failed"), 5000);
        return true;
    }
    if(strcmp(screen, "hold-feedback") == 0) {
        crazypod_desktop_hold_feedback_begin(
            LV_SYMBOL_AUDIO, 900);
        return true;
    }
    if(strcmp(screen, "headphone-wired") == 0 ||
       strcmp(screen, "headphone-over-ear") == 0 ||
       strcmp(screen, "headphone-airpods") == 0) {
        crazypod_state_set_headphone_popup_style(
            strcmp(screen, "headphone-airpods") == 0
                ? CRAZYPOD_HEADPHONE_POPUP_AIRPODS
                : strcmp(screen, "headphone-over-ear") == 0
                    ? CRAZYPOD_HEADPHONE_POPUP_OVER_EAR
                    : CRAZYPOD_HEADPHONE_POPUP_WIRED_EARBUDS);
        crazypod_headphone_popup_simulator_show();
        return true;
    }
    if(strcmp(screen, "lock") == 0 ||
       strcmp(screen, "lock-progress") == 0) {
        audio_stop();
        host->show_lock(false);
        if(strcmp(screen, "lock-progress") == 0)
            crazypod_lock_screen_simulator_set_progress(58);
        return true;
    }
    if(strcmp(screen, "lock-playing") == 0 ||
       strcmp(screen, "lock-paused") == 0 ||
       strcmp(screen, "lock-playing-progress") == 0) {
        if(!open_lock_playback_snapshot(
               host, strcmp(screen, "lock-paused") == 0))
            return false;
        if(strcmp(screen, "lock-playing-progress") == 0)
            crazypod_lock_screen_simulator_set_progress(58);
        return true;
    }
    if(strcmp(screen, "lock-playing-cjk") == 0)
        return open_lock_cjk_snapshot(host);
    if(strcmp(screen, "runtime-font-catalog") == 0)
        return open_runtime_font_catalog();
    if(strcmp(screen, "power") == 0) {
        host->show_power_prompt();
        return true;
    }
    if(strcmp(screen, "coverflow") == 0 ||
       strcmp(screen, "coverflow-power") == 0) {
        host->open_root_route(MUSIC_ROUTE_ALBUM_FLOW);
        if(strcmp(screen, "coverflow-power") == 0)
            host->show_power_prompt();
        return true;
    }
    if(sscanf(screen, "music-%d", &preview_index) == 1) {
        host->open_app(CRAZYPOD_APP_MUSIC);
        if(current_route() != NULL &&
           current_route()->route == MUSIC_ROUTE_MENU)
            select_bounded(host, preview_index);
        return true;
    }
    if(strcmp(screen, "music") == 0) {
        host->open_app(CRAZYPOD_APP_MUSIC);
        return true;
    }
    if(strcmp(screen, "podcasts") == 0) {
        host->open_app(CRAZYPOD_APP_PODCASTS);
        return true;
    }
    if(strcmp(screen, "settings") == 0) {
        host->open_app(CRAZYPOD_APP_SETTINGS);
        return true;
    }
    if(strcmp(screen, "settings-main-menu") == 0)
        host->open_root_route(SETTINGS_ROUTE_MAIN_MENU);
    else if(strcmp(screen, "settings-power") == 0)
        host->open_root_route(SETTINGS_ROUTE_POWER);
    else if(strcmp(screen, "settings-display") == 0)
        host->open_root_route(SETTINGS_ROUTE_DISPLAY);
    else if(strcmp(screen, "settings-sound") == 0)
        host->open_root_route(SETTINGS_ROUTE_SOUND);
    else if(strcmp(screen, "settings-playback") == 0)
        host->open_root_route(SETTINGS_ROUTE_PLAYBACK);
    else if(strcmp(screen, "settings-eq") == 0)
        host->open_root_route(SETTINGS_ROUTE_EQ_STUDIO);
    else if(strcmp(screen, "settings-controls") == 0)
        host->open_root_route(SETTINGS_ROUTE_CONTROLS);
    else if(strcmp(screen, "settings-main-menu-reorder") == 0) {
        host->open_root_route(SETTINGS_ROUTE_MAIN_MENU);
        host->activate_selected();
        host->move_selection(1);
    }
    else if(sscanf(screen,
                "settings-main-menu-reorder-%d",
                &preview_index) == 1) {
        host->open_root_route(SETTINGS_ROUTE_MAIN_MENU);
        select_bounded(host, preview_index);
        host->activate_selected();
        host->move_selection(1);
        host->activate_selected();
        host->move_selection(1);
    }
    else if(sscanf(screen,
                "settings-main-menu-actions-%d",
                &preview_index) == 1) {
        host->open_root_route(SETTINGS_ROUTE_MAIN_MENU);
        select_bounded(host, preview_index);
        host->activate_selected();
    }
    else if(strcmp(screen, "settings-language") == 0 ||
            strcmp(screen, "settings-language-receipt") == 0) {
        host->open_root_route(SETTINGS_ROUTE_MENU);
        select_bounded(host, item_count() - 1);
        host->activate_selected();
        if(strcmp(screen, "settings-language-receipt") == 0)
            host->activate_selected();
    }
    else if(strcmp(screen, "settings-reduce-motion") == 0) {
        host->open_root_route(SETTINGS_ROUTE_DISPLAY);
        select_bounded(host, item_count() - 1);
    }
    else if(strcmp(screen, "home-actions") == 0)
        host->open_root_route(HOME_ROUTE_ACTIONS);
    else if(strcmp(screen, "home-brightness") == 0)
        host->open_root_route(HOME_ROUTE_BRIGHTNESS);
    else if(strcmp(screen, "home-volume") == 0)
        host->open_root_route(HOME_ROUTE_VOLUME);
    else if(strcmp(screen, "now-playing-default") == 0) {
        host->open_root_route(MUSIC_ROUTE_NOW_PLAYING);
    }
    else if(strcmp(screen, "now-playing-default-media") == 0)
        return open_now_playing_default_media(host);
    else if(strcmp(screen, "now-playing-default-actions") == 0)
        return open_now_playing_default_actions(host);
    else if(strcmp(screen, "now-playing-default-mode") == 0)
        return open_now_playing_default_mode(host);
    else if(strcmp(screen, "now-playing-default-progress") == 0)
        return open_now_playing_default_progress(host);
    else if(strcmp(screen, "now-playing-default-lyrics") == 0)
        return open_now_playing_default_lyrics(host);
    else {
        fprintf(stderr,
                "CrazyPod unsupported music-only snapshot: %s\n",
                screen);
        return false;
    }
    return true;
}

#endif
