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
    # The real simulator sources need an independent Linux compile/link gate
    # when the macOS runner is queued; do not stage old full font packages.
    assert "linux_simulator_link:" in workflow
    snapshot_source = source(
        "apps/crazypod/ui/app/crazypod_simulator_snapshot.c"
    )
    assert '#include "string-extra.h"' in snapshot_source
    assert "strlcpy(buffer, actions, sizeof(buffer));" in snapshot_source
    legacy_photo = source("apps/crazypod/photos/crazypod_photo_catalog.c")
    assert "#ifdef SIMULATOR\n#include <strings.h>" in legacy_photo
    assert 'strcasecmp(extension, ".jpg")' in legacy_photo
    assert "name: Linux simulator link check" in workflow
    linux_job = workflow.split("  linux_simulator_link:", 1)[1].split(
        "  simulator:", 1
    )[0]
    assert "../tools/configure --target=ipod6g --type=s --sdl-threads" in linux_job
    assert "make -j2" in linux_job
    assert "test -x rockboxui" in linux_job
    assert "build-crazypod-runtime-fonts.sh" not in linux_job
    assert "runs-on: macos-latest" in workflow

    # The hardware package must not include all 47 x 4 CJK faces needed by
    # retired CPK apps. Simulator/font fixtures keep their original manifest.
    full_font_specs = {
        line.strip() for line in source(
            "tools/crazypod-runtime-font-specs.txt").splitlines()
        if line.strip() and not line.lstrip().startswith("#")
    }
    music_font_specs = {
        line.strip() for line in source(
            "tools/crazypod-runtime-font-music-only.txt").splitlines()
        if line.strip() and not line.lstrip().startswith("#")
    }
    assert len(music_font_specs) == 9, music_font_specs
    assert music_font_specs <= full_font_specs
    assert all(spec.startswith("system:") for spec in music_font_specs)
    for size in (8, 10, 12, 15, 16, 18):
        assert f"system:400:{size}" in music_font_specs
    for size in (8, 10, 16):
        assert f"system:700:{size}" in music_font_specs
        assert f"system:400:{size}" in music_font_specs, "SC Noto fallback"
    font_builder = source("tools/build-crazypod-runtime-fonts.sh")
    font_runtime = source("apps/crazypod/crazypod_runtime_font.c")
    assert 'CRAZYPOD_RUNTIME_FONT_SPECS:-' in font_builder
    assert 'CRAZYPOD_RUNTIME_FONT_SPECS="$CRAZYPOD_FONT_SPEC"' in hw
    assert '--spec-file "$CRAZYPOD_FONT_SPEC"' in hw
    assert 'tools/build-crazypod-runtime-fonts.sh' in sim
    assert 'CRAZYPOD_RUNTIME_FONT_MAX 24' in font_runtime
    assert 'CRAZYPOD_RUNTIME_FONT_MAX 48' in font_runtime
    usb_prompts = source("apps/crazypod/ui/shell/crazypod_system_prompts.c")
    for retired_usb in ("crazypod_books_", "crazypod_photos_",
                        "crazypod_miniapps_", "crazypod_organizer_",
                        "crazypod_customize_"):
        assert retired_usb not in usb_prompts, retired_usb
    for safety_path in (
        "prompts.storage_active = true;",
        "crazypod_artwork_suspend();",
        "crazypod_artwork_resume();",
        "crazypod_music_cancel_scan();",
        "crazypod_music_require_catalog_validation();",
        "usb_acknowledge(SYS_USB_CONNECTED_ACK, data);",
        "font_disable_all();",
        "font_enable_all();",
        "crazypod_music_library_schedule_rescan(",
    ):
        assert safety_path in usb_prompts, safety_path
    assert "remove(MEDIA_INVALID_PATH);" not in usb_prompts
    coordinator = source("apps/crazypod/ui/app/crazypod_choice_coordinator.c")
    for retired in ("crazypod_books_", "crazypod_customize_",
                    "crazypod_photos_", "crazypod_photo_",
                    "crazypod_wallpaper_"):
        assert retired not in coordinator, retired
    assert "crazypod_settings_feature_apply_choice(id, selected)" in coordinator
    assert "crazypod_now_playing_theme_select(selected)" in coordinator
    assert "return route == SETTINGS_ROUTE_MAIN_MENU_ACTIONS;" in coordinator
    app_input = source("apps/crazypod/ui/app/crazypod_app_input.c")
    rows = source("apps/crazypod/ui/app/crazypod_menu_rows.c")
    assert "crazypod_notes_feature_toggle_editor_field(" not in app_input
    assert "crazypod_miniapps_feature_close(" not in startup
    assert "crazypod_organizer_feature_stopwatch_running(" not in startup
    assert "crazypod_organizer_feature_workout_running(" not in rows
    assert "host.toggle_playback();" in app_input
    composition = source("apps/crazypod/ui/app/crazypod_composition.c")
    # Discontinued feature hosts must not register callbacks at startup.
    assert "crazypod_books_feature_configure_runtime(" not in composition
    assert "crazypod_photos_runtime_configure(" not in composition
    assert "crazypod_music_library_configure(&music);" in composition
    assert "crazypod_wallpaper_crop_runtime_configure(&crop);" not in composition
    assert "crazypod_photos_note_file_added();" not in source(
        "apps/crazypod/crazypod_screenshot.c"
    )
    assert "screen_dump_to_file(path)" in source(
        "apps/crazypod/crazypod_screenshot.c"
    )
    assert "crazypod_photos_feature_poll_media(" not in source(
        "apps/crazypod/ui/app/crazypod_playback.c"
    )
    assert "crazypod_customize_feature_poll_media(" not in source(
        "apps/crazypod/ui/app/crazypod_playback.c"
    )
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
    # Books/EPUB must not compile into the real iPod firmware. Keep
    # simulator-only sources and legacy disk content intact.
    reader_guard = manifest.split(
        "/* Retired reader compatibility: compiled only into the legacy simulator,", 1
    )[1].split("#endif", 1)[0]
    for reader_source in (
        "crazypod/crazypod_books.c",
        "crazypod/crazypod_epub.c",
        "crazypod/crazypod_book_cover.c",
        "crazypod/epub/crazypod_epub_html.c",
        "crazypod/ui/features/books/crazypod_books_feature.c",
        "crazypod/ui/features/books/crazypod_books_screen.c",
    ):
        assert reader_source in reader_guard, reader_source
    assert "#ifdef SIMULATOR\n/* Retired reader compatibility:" in manifest
    # The iPod ELF must not bring in the discontinued application engines.
    # Keep the original sources for simulator-only legacy diagnostics.
    retired_apps_guard = manifest.split(
        "/* Retired Notes/Photos/Organizer/Customize/MiniApps compatibility:", 1
    )[1].split("#endif", 1)[0]
    for retired_source in (
        "crazypod/crazypod_photos.c",
        "crazypod/crazypod_notes.c",
        "crazypod/crazypod_organizer.c",
        "crazypod/crazypod_workouts.c",
        "crazypod/crazypod_miniapps.c",
        "crazypod/crazypod_presets.c",
        "crazypod/miniapps/runtime/crazypod_miniapp_native_runtime.c",
        "crazypod/ui/features/photos/crazypod_photos_runtime.c",
        "crazypod/ui/features/miniapps/crazypod_miniapps_feature.c",
        "crazypod/ui/features/customize/crazypod_wallpaper_crop_runtime.c",
    ):
        assert retired_source in retired_apps_guard, retired_source
    assert "crazypod/crazypod_music.c" in manifest
    assert "crazypod/crazypod_screenshot.c" in manifest
    # A bounded bidirectional cache keeps visible artwork responsive but
    # avoids permanently reserving 51 x 2 x 128x128 pixel banks in BSS.
    art_header = source("apps/crazypod/crazypod_artwork.h")
    coverflow = source("apps/crazypod/crazypod_coverflow.c")
    assert "#define CRAZYPOD_COVERFLOW_ARTWORK_SLOTS 33" in art_header
    assert "#define FLOW_PREFETCH_AHEAD 16" in coverflow
    assert "#define FLOW_PREFETCH_BEHIND 16" in coverflow
    assert "#define FLOW_PREFETCH_VISIBLE 6" in coverflow
    assert "slot count must match the look-ahead window" in coverflow
    assert "candidate_number <= FLOW_PREFETCH_VISIBLE * 2" in coverflow
    assert "crazypod_artwork_load_cached_priority(" in coverflow
    artwork = source("apps/crazypod/crazypod_artwork.c")
    assert "static fb_data coverflow_pixels[CRAZYPOD_COVERFLOW_ARTWORK_SLOTS]" in artwork
    assert "[CRAZYPOD_ARTWORK_BANKS][CRAZYPOD_COVERFLOW_ARTWORK_SIZE *" in artwork
    # Native Now Playing's requested size is exactly the canonical 128px.
    # Shrink only the published double-buffer banks, never JPEG scratch.
    now_theme = source(
        "apps/crazypod/ui/features/now_playing/crazypod_now_playing_theme.c"
    )
    now_feature = source(
        "apps/crazypod/ui/features/now_playing/crazypod_now_playing_feature.c"
    )
    assert "CRAZYPOD_ARTWORK_CACHE_SIZE);" in now_theme
    assert "static int configured_source_size = CRAZYPOD_ARTWORK_CACHE_SIZE;" in now_feature
    assert "#define CRAZYPOD_NOW_ARTWORK_MAX_SIZE CRAZYPOD_ARTWORK_CACHE_SIZE" in art_header
    assert "#ifdef SIMULATOR\n#define CRAZYPOD_NOW_ARTWORK_MAX_SIZE 320" in art_header
    assert "#define CRAZYPOD_ARTWORK_MAX_SIZE 320" in artwork
    assert "static unsigned char decode_buffer[CRAZYPOD_DECODE_BUFFER_SIZE]" in artwork
    assert "static fb_data now_pixels[3][CRAZYPOD_ARTWORK_BANKS]" in artwork
    assert "crazypod/wallpaper/crazypod_wallpaper_store.c" in manifest
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
    # An incomplete font install must never launch a NULL-font desktop.
    runtime_font = source("apps/crazypod/crazypod_runtime_font.c")
    assert "return crazypod_runtime_fonts_ready();" in runtime_font
    assert "if(initialized)" not in runtime_font
    for spec in source("tools/crazypod-runtime-font-music-only.txt").splitlines():
        if spec.startswith("system:"):
            _, weight, size = spec.split(":")
            assert (
                f"crazypod_runtime_font_at_size_weight({size}, {weight})" in
                runtime_font
            ), f"Boot readiness did not pin shipped font {spec}"
    assert "if(!crazypod_runtime_font_init())" in startup
    assert "(void)font_recovery_wait(display);" in startup
    recovery = startup.split("static lv_obj_t *font_recovery_screen(void)", 1)[1]
    recovery = recovery.split("static lv_obj_t *create_boot_screen(void)", 1)[0]
    assert "&lv_font_montserrat_12" in recovery
    assert "crazypod_runtime_font_at_size(" not in recovery
    assert "crazypod_usb_prompt_set_ui_ready(true);" in recovery
    assert "crazypod_usb_prompt_finish(USB_MODE_MASS_STORAGE);" in recovery
    assert "usb_acknowledge(SYS_USB_CONNECTED_ACK, data);" in recovery
    assert "font_disable_all();" in recovery
    assert "font_enable_all();" in recovery
    # Disconnect must not re-enable descriptors for removed/altered fonts.
    font_disconnected = usb_prompts.split(
        "void crazypod_system_prompts_usb_disconnected(void)", 1
    )[1].split("void crazypod_system_prompts_power_off(void)", 1)[0]
    assert font_disconnected.index("crazypod_runtime_fonts_storage_valid()") < (
        font_disconnected.index("font_enable_all()")
    )
    assert "shutdown_hw(SHUTDOWN_REBOOT);" in font_disconnected
    assert "return;" in font_disconnected
    assert "font_disable_all();" in usb_prompts
    assert "storage_validation_coverage" in runtime_font
    assert "memcmp(slot->coverage, storage_validation_coverage," in runtime_font
    assert "assert(!crazypod_runtime_fonts_storage_valid());" in font_host_test
    assert "if(storage_active)" in recovery
    widgets = source("apps/crazypod/ui/presentation/crazypod_ui_widgets.c")
    assert widgets.count("return face != NULL ? face : font;") == 3
    font_host_test = source("tests/crazypod_runtime_font_host_test.c")
    assert 'sc-system-700-10.fnt' in font_host_test
    assert "assert(!crazypod_runtime_font_init());" in font_host_test
    assert "assert(crazypod_runtime_font_init());" in font_host_test
    assert "assert(loaded_count() == 15);" in font_host_test
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
