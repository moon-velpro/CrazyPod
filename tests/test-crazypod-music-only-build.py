#!/usr/bin/env python3
"""Guard the public iPod-only firmware and distribution against old app baggage."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def source(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def verify() -> None:
    manifest = source("apps/SOURCES")
    apps_make = source("apps/apps.make")
    hw = source("build-hw.sh")
    sim = source("build-sim.sh")
    workflow = source(".github/workflows/build.yml")
    boot = source("apps/crazypod/crazypod_main.c")
    launcher = source("apps/crazypod/ui/app/crazypod_app_launcher.c")
    desktop = source("apps/crazypod/ui/shell/crazypod_desktop.c")
    native_home = source("apps/crazypod/ui/shell/crazypod_desktop_native.c")
    retired_icons = source("apps/crazypod/crazypod_icons.c")
    present = source("apps/crazypod/crazypod_frameclock.c")
    present_header = source("apps/crazypod/crazypod_frameclock.h")
    home_input = source("apps/crazypod/ui/shell/crazypod_home_input.c")
    home_actions = source("apps/crazypod/ui/shell/crazypod_home_actions.c")
    capsule = source("apps/crazypod/ui/shell/crazypod_now_capsule.c")
    native_menu = source("apps/crazypod/ui/presentation/crazypod_menu_screen.c")
    native_refresh = source("apps/crazypod/ui/presentation/crazypod_menu_list.c")
    preview = source("apps/crazypod/ui/app/crazypod_menu_preview.c")
    now_screen = source(
        "apps/crazypod/ui/features/now_playing/crazypod_now_screen.c"
    )
    now_header = source(
        "apps/crazypod/ui/features/now_playing/crazypod_now_screen.h"
    )
    now_presentation = source(
        "apps/crazypod/ui/features/now_playing/crazypod_now_presentation.c"
    )
    now_presentation_header = source(
        "apps/crazypod/ui/features/now_playing/crazypod_now_presentation.h"
    )
    liquid = source("apps/crazypod/ui/presentation/crazypod_liquid_music.h")
    glass_overlay = source("apps/crazypod/ui/presentation/crazypod_overlay_glass.c")
    glass_panel = source("apps/crazypod/ui/presentation/crazypod_glass_panel.c")
    glass_sampler = source("apps/crazypod/ui/presentation/crazypod_glass_sampler.c")
    glass_sampler_header = source(
        "apps/crazypod/ui/presentation/crazypod_glass_sampler.h"
    )
    image = source("apps/crazypod/crazypod_image.c")
    runtime = source("apps/crazypod/ui/app/crazypod_runtime_services.c")
    feature_input = source("apps/crazypod/ui/app/crazypod_feature_input.c")
    query = source("apps/crazypod/ui/navigation/crazypod_route_query.c")
    renderer = source("apps/crazypod/ui/app/crazypod_route_renderer.c")
    registry = source("apps/crazypod/ui/navigation/crazypod_route_registry.c")
    route_actions = source("apps/crazypod/ui/app/crazypod_route_actions.c")
    startup = source("apps/crazypod/crazypod_ui.c")
    theme = source(
        "apps/crazypod/ui/features/now_playing/crazypod_now_playing_theme.c"
    )

    # No legacy app catalog work or disk mutation before first music scan.
    for legacy in ("crazypod_notes_init();", "crazypod_books_init();",
                   "crazypod_workouts_init();"):
        assert legacy not in boot, f"Legacy boot hook still active: {legacy}"
    assert boot.count("crazypod_music_init();") == 2
    assert boot.count("crazypod_appearance_load();") == 2, (
        "Persisted appearance must still load in both startup paths"
    )
    assert "crazypod_presets_load();" not in boot, (
        "Legacy DIY presets must not be scanned at boot"
    )
    # These IDs persist for compatibility, but the launcher has no paths to
    # initialize any retired app from the product navigation.
    for legacy in ("CRAZYPOD_APP_PHOTOS", "CRAZYPOD_APP_CUSTOMIZE",
                   "CRAZYPOD_APP_WORKOUTS", "CRAZYPOD_APP_EXTRAS",
                   "CRAZYPOD_APP_NOTES", "CRAZYPOD_APP_BOOKS",
                   "CRAZYPOD_APP_MINI_APPS", "CRAZYPOD_APP_GAMEBOY",
                   "CRAZYPOD_APP_CLOCK", "CRAZYPOD_APP_CONTACTS",
                   "CRAZYPOD_APP_CALENDAR", "CRAZYPOD_APP_STOPWATCH"):
        assert f"case {legacy}:" not in launcher, f"Retired app launch: {legacy}"
    for active in ("CRAZYPOD_APP_MUSIC", "CRAZYPOD_APP_PODCASTS",
                   "CRAZYPOD_APP_SETTINGS", "CRAZYPOD_APP_SHUFFLE",
                   "CRAZYPOD_APP_LOCK"):
        assert f"case {active}:" in launcher, f"Missing critical action: {active}"
    assert "crazypod_product_launcher_allowed(id)" in launcher
    composition = source("apps/crazypod/ui/app/crazypod_composition.c")
    # Discontinued feature hosts must not register callbacks at startup.
    assert "crazypod_books_feature_configure_runtime(" not in composition
    assert "crazypod_photos_runtime_configure(" not in composition
    assert "crazypod_music_library_configure(&music);" in composition
    assert "crazypod_wallpaper_crop_runtime_configure(&crop);" in composition
    assert "crazypod_now_playing_overlay_configure(&overlay);" in composition
    for retired_preview in ("crazypod_books_feature_render_preview(",
                            "crazypod_notes_feature_render_preview(",
                            "crazypod_photos_feature_render_preview(",
                            "crazypod_organizer_feature_render_preview(",
                            "crazypod_customize_feature_render_preview(",
                            "crazypod_extras_preview_render("):
        assert retired_preview not in preview, retired_preview
    assert "crazypod_music_feature_render_item_preview(" in preview
    assert "crazypod_settings_feature_render_preview(" in preview

    # Legacy routes stay addressable as old persisted enum values, but
    # can never dispatch into deleted-app activate/confirm paths.
    assert "crazypod_route_registry_product_allowed(state->route)" in route_actions
    assert "crazypod_books_feature_activate(" not in route_actions
    assert "crazypod_notes_feature_activate(" not in route_actions
    assert "crazypod_photos_feature_activate(" not in route_actions
    assert "crazypod_organizer_feature_activate(" not in route_actions
    assert "crazypod_miniapps_feature_activate(" not in route_actions
    assert "crazypod_customize_feature_activate(" not in route_actions
    assert "crazypod_books_feature_confirm(" not in startup
    assert "crazypod_notes_feature_confirm(" not in startup
    assert "crazypod_organizer_feature_confirm(" not in startup
    assert "crazypod_route_actions_confirm_photos(" not in startup
    assert "crazypod_settings_feature_activate(state, &actions)" in route_actions
    assert "crazypod_music_feature_activate(" in route_actions

    # MPEG-1/2 decoder + video catalog/engines are excluded from the ELF,
    # not merely hidden from Home. The only linked compatibility layer is
    # a no-I/O stub whose behavior has a separate compiled C99 host test.
    assert "crazypod/crazypod_videos_disabled.c" in manifest
    for retired_source in (
        "crazypod/crazypod_videos.c",
        "crazypod/crazypod_video_mpeg_misc.c",
        "crazypod/video/",
        "plugins/mpegplayer/",
    ):
        assert retired_source not in manifest, (
            f"Removed video backend still linked: {retired_source}"
        )
    assert "crazypod_video_disabled_host_test.c" in source(
        "tests/run-crazypod-ui-host-tests.sh"
    )

    # The old horizontal 160px BMP carousel and 120px interpolation
    # cache are not part of the vertical Home. Keep modal underlay support
    # while preventing multi-megabyte BSS regressions and boot-time BMP I/O.
    assert "static struct icon_slot slots[" not in retired_icons
    assert "ICON_BYTES" not in retired_icons
    assert "load_icon(" not in retired_icons
    assert "open(" not in retired_icons
    assert "crazypod_icons_init();" not in startup
    assert "static uint8_t scaled_icons" not in native_home
    assert "static fb_data backdrop[" not in native_home
    assert "prepare_scaled_icon(" not in native_home
    assert "modal_underlay[LCD_WIDTH * NATIVE_HEIGHT]" in native_home
    assert "crazypod_desktop_native_create_modal_underlay(" in native_home
    assert "crazypod_desktop_native_preserve_modal_underlay(" in native_home
    assert "crazypod_desktop_native_prepare_modal(" in native_home

    # A real Click Wheel detent must only refresh old/new Home rows, not
    # rebuild all four labels or trigger repeated screen launches.
    detent_helper = desktop.split("static void update_selection_row(int index)", 1)[1].split(
        "static void update_selection_chrome(void)", 1
    )[0]
    assert "lv_obj_set_style_border_width" not in detent_helper
    assert "lv_obj_set_style_border_color" not in detent_helper
    assert "CP_LV_LABEL_SET_TEXT" not in detent_helper
    assert desktop.count("update_selection_row(previous);") == 2
    assert desktop.count("update_selection_row(selected);") == 2
    assert "CP_LV_LABEL_SET_TEXT(home_labels[index]" in desktop
    assert "event->base == BUTTON_SELECT && !event->repeated" in home_input
    assert "event->release" in home_input
    assert "BUTTON_SELECT | BUTTON_REPEAT" in source(
        "tests/crazypod_home_input_host_test.c"
    )
    assert "#define CRAZYPOD_TARGET_FPS 50" in source(
        "apps/crazypod/crazypod_frameclock.h"
    )
    assert desktop.count("crazypod_present_note_home_selection();") == 2
    assert "home_selection_last_latency_us" in present_header
    assert "home_selection_over_50ms" in present_header
    assert "home_selection_start_us = crazypod_monotonic_usec();" in present
    assert "if(!submitted)" in present
    assert "if(home_selection_pending && home_selection_frame_ready)" in present
    assert "crazypod_present_home_selection_frame_flushed();" in startup
    assert "crazypod_present_home_selection_waiting_for_flush()" in startup
    assert "crazypod_present_queue_full();" not in desktop, (
        "Home wheel must not submit the old framebuffer before LVGL redraw"
    )
    assert "test_home_selection_waits_for_real_lvgl_flush" in source(
        "tests/crazypod_frameclock_host_test.c"
    )


    # Home quick controls reuse the existing static, two-pass RGB565
    # backdrop sampler. The material is prepared once per modal change,
    # never from wheel movement or rendering an animation frame.
    assert "CRAZYPOD_GLASS_HOME_CONTROLS" in glass_sampler_header
    assert "GLASS_HOME_TINT_COLOR" in glass_sampler
    assert "lv_obj_t *crazypod_overlay_glass_home_controls_panel(" in glass_overlay
    assert "crazypod_image_render_glass_rgb565(" in glass_overlay
    assert "crazypod_overlay_glass_home_controls_panel(" in home_actions
    assert "lv_obj_t *top_glint" in glass_panel
    assert "lv_obj_t *bottom_rim" in glass_panel
    assert "crazypod_image_render_glass_rgb565(" in image
    assert "for(offset = -2; offset <= 2; ++offset)" in image
    hot_path = home_actions.split("static void refresh_action_selection(void)", 1)[1].split(
        "static void show_action_list(void)", 1
    )[0]
    assert "crazypod_overlay_glass" not in hot_path
    assert "crazypod_image_render_glass_rgb565" not in desktop
    assert "lv_obj_set_style_blur_backdrop" not in home_actions, (
        "No per-frame software backdrop blur in Home controls"
    )

    # The first production pass implements the approved minimal
    # Apple-like direction as real LVGL, not a mockup: clean full-width
    # Music/Settings roots, quiet Home selection and a single glass mini
    # player. No new navigation action, audio thread or wheel polling.
    assert "CRAZYPOD_LM_NATIVE_BG" in liquid
    assert "CRAZYPOD_LM_HOME_IDLE_OPA          0" in liquid
    assert "CRAZYPOD_PRODUCT_HOME_VIEWPORT_HEIGHT" in source(
        "apps/crazypod/crazypod_product_geometry.h"
    )
    assert "home_viewport = lv_obj_create(screen);" in desktop
    assert "home_reel = lv_obj_create(home_viewport);" in desktop
    assert "lv_obj_set_style_clip_corner(home_viewport, true, 0)" in desktop
    assert "lv_obj_set_y(" in desktop
    assert "crazypod_home_reel_offset_y(selected_app)" in desktop
    assert desktop.count("update_home_vertical_scroll();") == 3
    assert "selected_app = CRAZYPOD_HOME_MUSIC;" in desktop
    assert "home_entry_subtitle(" in desktop
    assert "home_private_badges[index]" in desktop
    assert "home_fallback_glyphs[index]" in desktop
    assert "home_fallback_glyphs[i] = symbol;" in desktop
    assert "(selected ? 22 : 17) * LV_SCALE_NONE / 14" in desktop
    assert "crazypod_iphone_app_icon(" in desktop
    assert "crazypod_image_render_glass_rgb565(" not in desktop
    assert "native_root =" in renderer
    assert "? CRAZYPOD_LM_NATIVE_BG" in renderer
    assert "? CRAZYPOD_LM_NATIVE_INK" in renderer
    assert "state->route == MUSIC_ROUTE_MENU" in native_menu
    assert "state->route == SETTINGS_ROUTE_MENU" in native_menu
    assert "native_root ? LCD_WIDTH - 24" in native_menu
    assert "native_root ? LV_SYMBOL_RIGHT" in native_menu
    assert "bool native_root" in native_refresh
    assert "CRAZYPOD_LM_NATIVE_SELECTED" in native_refresh
    assert "lv_obj_set_style_bg_grad_dir" in native_refresh
    assert "Full-width native root lists do not need album cards" in preview
    assert "No preview pane exists in these native full-width lists" in preview

    # Native Quick Controls show levels, not three colored orbs.
    assert "static int action_level_percent(enum home_action action)" in home_actions
    assert "lv_bar_create(actions.panel)" in home_actions
    assert "lv_bar_set_value(actions.level_bar, value, LV_ANIM_OFF)" in home_actions
    assert "lv_arc_create(" not in home_actions
    assert "action_level_percent(action)" in home_actions
    assert "crazypod_overlay_glass_home_controls_panel(" in home_actions

    # Now Playing retains real artwork, lyrics, status and native physical
    # controls; it does not add touch input or synchronous image filters.
    for glyph in ("LV_SYMBOL_PREV", "LV_SYMBOL_NEXT", "LV_SYMBOL_PAUSE"):
        assert glyph in now_screen
    assert "lv_obj_t *transport_status;" in now_header
    assert "update_transport_play_state();" in now_screen
    assert "crazypod_lm_album_shade(palette.primary)" in now_presentation
    assert "crazypod_artwork_palette_extract(artwork, &palette)" in now_presentation
    assert "shade_colors[bank] = album_shade(artwork);" in now_presentation
    assert "shade_colors[bank]" in now_presentation
    assert "uint32_t *shade_color" in now_presentation_header
    assert "uint32_t shade_color = CRAZYPOD_NOW_SHADE_COLOR;" in now_screen
    assert "&content_color, &shade_color" in now_screen
    assert "shade_color, CRAZYPOD_NOW_SHADE_OPA" in now_screen
    transport_region = now_screen.split(
        "view->progress_marker = make_box(", 1
    )[1].split("void crazypod_now_screen_reset(", 1)[0]
    assert "COLOR_WHITE" not in transport_region, (
        "Artwork-adaptive background requires matching transport/time contrast"
    )
    assert transport_region.count("content_color") >= 6
    assert "crazypod_lm_album_shade(" in liquid
    assert "lv_obj_add_event_cb(view->wave_surface" in now_screen
    assert "lv_obj_add_flag(capsule.spectrum, LV_OBJ_FLAG_HIDDEN)" in capsule
    assert "capsule.play_symbol" in capsule
    assert "lv_obj_has_flag(capsule.spectrum, LV_OBJ_FLAG_HIDDEN)" in capsule

    # The route ABI is stable, but all retired dispatch paths are inert.
    assert "crazypod_route_registry_product_allowed(" in registry
    assert "crazypod_route_registry_product_allowed(route)" in launcher
    assert "crazypod_route_registry_product_allowed(route)" in route_actions
    assert "crazypod_route_registry_product_allowed(state->route)" in route_actions
    for retired in (
        "CRAZYPOD_FEATURE_BOOKS", "CRAZYPOD_FEATURE_NOTES",
        "CRAZYPOD_FEATURE_PHOTOS", "CRAZYPOD_FEATURE_ORGANIZER",
        "CRAZYPOD_FEATURE_CUSTOMIZE", "CRAZYPOD_FEATURE_MINIAPPS",
    ):
        assert retired not in feature_input, (
            f"Retired app input binding still compiled: {retired}"
        )
        assert retired not in query, (
            f"Retired app menu query still compiled: {retired}"
        )
        assert retired not in renderer, (
            f"Retired app renderer still compiled: {retired}"
        )
    for retained in ("CRAZYPOD_FEATURE_MUSIC",
                     "CRAZYPOD_FEATURE_NOW_PLAYING",
                     "CRAZYPOD_FEATURE_SETTINGS"):
        assert retained in feature_input and retained in query
        assert retained in renderer

    # No old app runtime may be initialized, scanned, or polled on boot.
    for abandoned_call in (
        "crazypod_miniapps_feature_rescan();",
        "crazypod_miniapp_alarm_initialize();",
        "crazypod_miniapp_alarm_tick(",
        "crazypod_miniapps_feature_service(",
        "crazypod_organizer_feature_service(",
        "crazypod_books_feature_service_reader(",
        "crazypod_photos_runtime_service(",
        "crazypod_route_actions_service_notes();",
    ):
        assert abandoned_call not in runtime, (
            f"Retired runtime still polled: {abandoned_call}"
        )
    # Never enumerate an old photo/video library during music-only boot.
    for retired_boot_hook in (
        "crazypod_photos_init();",
        "crazypod_videos_init();",
        "crazypod_photos_feature_initialize_media();",
        "crazypod_customize_feature_initialize_media();",
    ):
        assert retired_boot_hook not in startup, (
            f"Retired Media/Customize startup hook: {retired_boot_hook}"
        )
    assert "crazypod_wallpaper_init();" in startup, (
        "Existing user wallpapers must remain loadable"
    )
    assert "crazypod_artwork_init();" in startup, (
        "Music artwork cache startup was accidentally removed"
    )
    for abandoned_call in (
        "crazypod_miniapps_feature_initialize_runtime();",
        "crazypod_miniapps_feature_initialize();",
    ):
        assert abandoned_call not in startup, (
            f"Retired MiniApp startup hook: {abandoned_call}"
        )
    assert "CRAZYPOD_WAIT_BLOCK, crazypod_state_wait_ticks()" in runtime, (
        "LCD-off scheduler must not force 1Hz polling after alarm removal"
    )
    assert "crazypod_now_playing_theme_enabled(void)" in theme
    assert "return false; /* Always render the native Now Playing screen. */" in theme
    assert "crazypod_miniapps.h" not in theme
    assert "THEME_CONFIG" not in theme

    assert "crazypod/gameboy/" not in manifest, "Game Boy linked into firmware"
    assert "crazypod_gameboy_screen.c" not in manifest
    assert "/gameboy/crazypod_gameboy.make" not in apps_make
    assert "miniapps/miniapps.make" not in apps_make
    assert "npm ci" not in hw and "npm ci" not in sim
    assert "miniapp-builder/src/cli.mjs" not in hw
    assert "miniapp-builder/src/cli.mjs" not in sim
    assert "dist/miniapps" not in hw
    assert "dist/miniapps" not in workflow
    assert "MiniApps/Games" not in hw
    assert "MiniApps/Games" not in sim
    assert "Music Podcasts" in hw
    # The music-only firmware must not distribute retired 160px BMP art.
    assert "assets/crazypod-icons" not in hw
    assert "assets/crazypod-icons" not in sim
    assert ".rockbox/crazypod/icons" not in hw
    assert 'cp -R ../assets/crazypod-icons' not in sim
    assert 'cp ../assets/crazypod/default-home.bmp' in hw
    assert 'cp ../assets/crazypod/default-home.bmp' in sim
    assert 'CrazyPod: linked ELF text/data/bss (bytes)' in hw
    assert "CrazyPod-6G.zip" in hw
    assert not list((ROOT / "dist" / "miniapps").glob("*.cpk")), (
        "Retired CPK apps should not be checked into the product tree"
    )

    print("CrazyPod music-only build/package scope passed")


if __name__ == "__main__":
    verify()
