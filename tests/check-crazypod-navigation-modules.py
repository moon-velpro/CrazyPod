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
    assert "crazypod_now_playing_theme_handle_input" not in inputs

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
