#!/usr/bin/env python3
"""Enforce music-only navigation module boundaries at source level.

These are architecture checks, not a substitute for compiling or hardware
testing. Legacy source files elsewhere are removed in later module slices.
"""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
UI = ROOT / "apps/crazypod/ui"

REGISTRY = UI / "navigation/crazypod_route_registry.c"
QUERY = UI / "navigation/crazypod_route_query.c"
INPUT = UI / "app/crazypod_feature_input.c"
APP_INPUT = UI / "app/crazypod_app_input.c"
APP_INPUT_HOST = UI / "app/crazypod_app_input.h"
UI_ROOT = ROOT / "apps/crazypod/crazypod_ui.c"
RENDERER = UI / "app/crazypod_route_renderer.c"
LAUNCHER = UI / "app/crazypod_app_launcher.c"
ROUTES = UI / "navigation/crazypod_ui_routes.h"
SOURCES = ROOT / "apps/SOURCES"

LIVE_FEATURES = {"MUSIC", "NOW_PLAYING", "SETTINGS"}
RETIRED_FEATURES = {
    "BOOKS", "NOTES", "PHOTOS", "ORGANIZER", "CUSTOMIZE", "MINIAPPS"
}
RETIRED_MODULES = (
    "books", "notes", "photos", "organizer", "customize", "miniapps"
)


def contents(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def check() -> None:
    registry = contents(REGISTRY)
    query = contents(QUERY)
    inputs = contents(INPUT)
    app_input = contents(APP_INPUT)
    app_input_host = contents(APP_INPUT_HOST)
    ui_root = contents(UI_ROOT)
    renderer = contents(RENDERER)
    launcher = contents(LAUNCHER)
    routes = contents(ROUTES)
    sources = contents(SOURCES)

    for path, text in ((REGISTRY, registry), (QUERY, query), (INPUT, inputs),
                       (RENDERER, renderer), (APP_INPUT, app_input)):
        for owner in RETIRED_MODULES:
            assert f"features/{owner}/" not in text, (path, owner)
        for owner in RETIRED_FEATURES:
            assert f"CRAZYPOD_FEATURE_{owner}" not in text, (path, owner)
        assert "GAMEBOY_ROUTE" not in text, path

    for name in LIVE_FEATURES:
        assert f"CRAZYPOD_FEATURE_{name}" in registry
    assert "crazypod_feature_get(descriptor->feature->id)" in registry
    assert "if(!crazypod_ui_route_supported(route))" in registry
    assert "if(!crazypod_ui_route_supported(route))" in contents(
        UI / "navigation/crazypod_ui_routes.c"
    )
    assert "crazypod_ui_route_supported(route)" in launcher
    assert "music_only_root_route" not in launcher
    assert "crazypod_route_registry_get(state->route)" in renderer
    actions = contents(UI / "app/crazypod_route_actions.c")
    # An unhandled Settings / Home route must never fall through into a
    # music-playback failure notification. Only song-bearing pages can do so.
    selected_track = actions.split(
        "static void play_selected_track(", 1
    )[1].split("static void persist_main_menu(", 1)[0]
    assert re.search(r"default:[^{}]*\\breturn;", selected_track), (
        "unhandled routes must return before playback failure"
    )
    for track_route in (
        "MUSIC_ROUTE_ALL", "MUSIC_ROUTE_SONGS",
        "MUSIC_ROUTE_PLAYLIST_SONGS", "MUSIC_ROUTE_ARTIST_SONGS",
        "MUSIC_ROUTE_ALBUM_SONGS", "MUSIC_ROUTE_QUEUE",
        "MUSIC_ROUTE_SEARCH_RESULTS",
    ):
        assert track_route in selected_track, track_route
    assert "crazypod_now_playing_theme_handle_input" not in inputs
    assert "handle_confirmation" not in app_input_host
    assert "host.handle_confirmation" not in app_input
    assert "static bool handle_confirmation(" not in ui_root
    for retired_entry in (
        "crazypod_photos_init(", "crazypod_videos_init(",
        "crazypod_miniapps_feature_initialize(",
        "crazypod_miniapps_feature_initialize_runtime(",
        "crazypod_photos_feature_initialize_media(",
        "crazypod_customize_feature_initialize_media(",
        "crazypod_miniapp_repro_service(",
    ):
        assert retired_entry not in ui_root, retired_entry

    # All supported routes must be declared in the sparse product table.
    body = registry.split("routes[CRAZYPOD_ROUTE_COUNT] = {", 1)[1].split(
        "\n};", 1
    )[0]
    route_names = re.findall(
        r"(?:FEATURE_ROUTE(?:_FLAGS)?|SHELL_ROUTE)\(\s*"
        r"([A-Z_]+(?:[A-Z0-9_]*))", body
    )
    assert route_names, "route descriptor table is empty"
    assert len(route_names) == len(set(route_names)), "duplicate route"
    enum_body = routes.split("enum crazypod_route {", 1)[1].split("};", 1)[0]
    historic_routes = set(
        re.findall(r"^\s*([A-Z][A-Z0-9_]+)\s*,", enum_body, re.M)
    )
    assert set(route_names) <= historic_routes, "undeclared route IDs"
    assert set(route_names) == {
        name for name in historic_routes
        if (
            name.startswith("MUSIC_ROUTE_")
            or name.startswith("SETTINGS_ROUTE_")
            or name.startswith("HOME_ROUTE_")
            or name == "PODCASTS_ROUTE_MENU"
        )
    }

    # The shipping image must not link any retired independent app,
    # installed CPK runtime, or software video decoder.
    shipping = {
        line.strip() for line in sources.splitlines()
        if line.startswith("crazypod/") or
        line.startswith("plugins/mpegplayer/")
    }
    retired_prefixes = (
        "crazypod/miniapps/",
        "crazypod/photos/",
        "crazypod/video/",
        "crazypod/epub/",
        "crazypod/ui/features/miniapps/",
        "crazypod/ui/features/photos/",
        "crazypod/ui/features/books/",
        "crazypod/ui/features/notes/",
        "crazypod/ui/features/organizer/",
        "crazypod/ui/features/customize/",
        "plugins/mpegplayer/",
    )
    retired_modules = {
        "crazypod/crazypod_miniapps.c",
        "crazypod/crazypod_miniapp_input.c",
        "crazypod/crazypod_miniapp_font.c",
        "crazypod/crazypod_miniapp_asset_font.c",
        "crazypod/crazypod_photos.c",
        "crazypod/crazypod_videos.c",
        "crazypod/crazypod_video_mpeg_misc.c",
        "crazypod/crazypod_books.c",
        "crazypod/crazypod_notes.c",
        "crazypod/crazypod_organizer.c",
        "crazypod/crazypod_workouts.c",
        "crazypod/crazypod_epub.c",
        "crazypod/crazypod_book_cover.c",
        "crazypod/ui/app/crazypod_miniapp_repro.c",
        "crazypod/ui/shell/crazypod_extras_preview.c",
        "crazypod/wallpaper/crazypod_wallpaper_crop_engine.c",
    }
    for name in shipping:
        assert not name.startswith(retired_prefixes), name
        assert name not in retired_modules, name

    for path in (
        UI_ROOT,
        UI / "app/crazypod_route_actions.c",
        UI / "app/crazypod_choice_coordinator.c",
        UI / "app/crazypod_composition.c",
        UI / "app/crazypod_menu_preview.c",
        UI / "app/crazypod_playback.c",
        UI / "shell/crazypod_system_prompts.c",
        UI / "app/crazypod_simulator_snapshot.c",
    ):
        text = contents(path)
        for retired in (
            "crazypod_miniapps_feature_",
            "crazypod_photos_feature_",
            "crazypod_books_feature_",
            "crazypod_notes_feature_",
            "crazypod_organizer_feature_",
            "crazypod_customize_feature_",
        ):
            assert retired not in text, (path, retired)

    # Audit every compiled CrazyPod translation unit, not only UI entry
    # points. A source-list refactor is incomplete if another live module
    # still calls an unlinked legacy domain.
    legacy_reference = re.compile(
        r"crazypod_(?:notes|books|epub|photos|videos|organizer|"
        r"workouts|miniapps|miniapp|presets)(?:_|\()|"
        r"/features/(?:notes|books|photos|organizer|customize|miniapps)/"
    )
    product_sources = sorted(
        name for name in shipping
        if name.startswith("crazypod/") and name.endswith(".c")
    )
    assert product_sources, "empty product source list"
    for name in product_sources:
        source_path = ROOT / "apps" / name
        assert source_path.is_file(), f"missing compiled source: {name}"
        match = legacy_reference.search(contents(source_path))
        assert match is None, f"orphaned retired API in {name}: {match.group()}"

    for module in (
        "ui/navigation/crazypod_route_registry.c",
        "ui/navigation/crazypod_route_query.c",
        "ui/app/crazypod_feature_input.c",
        "ui/app/crazypod_route_renderer.c",
        "ui/app/crazypod_app_input.c",
    ):
        assert f"crazypod/{module}" in sources, module


if __name__ == "__main__":
    check()
    print("Music-only navigation module boundaries passed")
