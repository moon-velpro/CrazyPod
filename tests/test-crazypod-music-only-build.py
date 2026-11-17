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
    assert "CrazyPod-6G.zip" in hw
    assert not list((ROOT / "dist" / "miniapps").glob("*.cpk")), (
        "Retired CPK apps should not be checked into the product tree"
    )

    print("CrazyPod music-only build/package scope passed")


if __name__ == "__main__":
    verify()
