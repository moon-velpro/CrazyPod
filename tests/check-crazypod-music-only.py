#!/usr/bin/env python3
"""Guard music-only Home and shipping payload against legacy app regression."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]


def read(path):
    return (ROOT / path).read_text(encoding="utf-8")


def main():
    apps = read("apps/crazypod/crazypod_apps.c")
    catalog = read("apps/crazypod/ui/shell/crazypod_app_catalog.c")
    launcher = read("apps/crazypod/ui/app/crazypod_app_launcher.c")
    paths = read("apps/crazypod/crazypod_icons.c")
    header = read("apps/crazypod/crazypod_apps.h")
    icon_header = read("apps/crazypod/crazypod_icons.h")
    expected = [
        "MUSIC", "PODCASTS", "SHUFFLE", "LOCK", "SETTINGS"
    ]

    order = (apps.split("static const uint8_t default_order", 1)[1]
             .split("{", 1)[1].split("};", 1)[0])
    assert re.findall(r"CRAZYPOD_APP_([A-Z_]+)", order) == expected
    assert re.findall(r"\{ CRAZYPOD_APP_([A-Z_]+),", catalog) == expected
    assert "#define CRAZYPOD_APP_COUNT 5" in header
    assert "#define CRAZYPOD_APP_DISK_COUNT 17" in header
    assert "#define CRAZYPOD_ICON_COUNT 5" in icon_header
    icon_paths = paths.split("static const char *const app_paths", 1)[1].split("};", 1)[0]
    for icon in ("music", "podcasts", "shuffle", "screen_lock", "settings"):
        assert f'"{icon}"' in icon_paths
    assert icon_paths.count('"') == 10
    assert "music_only_root_route(route)" in launcher
    routes = read("apps/crazypod/ui/navigation/crazypod_ui_routes.h")
    stack = read("apps/crazypod/ui/navigation/crazypod_ui_routes.c")
    registry = read("apps/crazypod/ui/navigation/crazypod_route_registry.c")
    assert "crazypod_ui_route_supported" in routes
    assert "if(!crazypod_ui_route_supported(route))" in stack
    assert "if(!crazypod_ui_route_supported(route))" in registry
    actions = read("apps/crazypod/ui/app/crazypod_route_actions.c")
    push = actions.split("void crazypod_route_actions_push_selected(", 1)[1].split(
        "void crazypod_route_actions_push(", 1
    )[0]
    # Legacy choice dispatch must not run before the retired-route gate.
    assert push.index("if(!crazypod_ui_route_supported(route))") < push.index(
        "if(route == DIY_ROUTE_NOW_PLAYING_THEMES)"
    )
    for entry in (
        "void crazypod_route_actions_activate(long now)",
        "void crazypod_route_actions_activate_overlay_state(",
        "void crazypod_route_actions_move(int direction, long now)",
    ):
        body = actions.split(entry, 1)[1].split("\n}", 1)[0]
        assert "crazypod_ui_route_supported(" in body, entry
    for retired in (
        "APP_MINI_APPS", "APP_PHOTOS", "APP_BOOKS", "APP_NOTES",
        "APP_CONTACTS", "APP_CALENDAR", "APP_WORKOUTS", "APP_STOPWATCH",
        "APP_EXTRAS", "APP_CUSTOMIZE",
    ):
        assert "case CRAZYPOD_" + retired not in launcher, retired
    for script in ("build-hw.sh", "build-sim.sh"):
        content = read(script)
        assert "for icon in music podcasts shuffle screen_lock settings; do" in content
        assert "crazypod/icons" in content
    hw = read("build-hw.sh")
    sim = read("build-sim.sh")
    make = read("apps/apps.make")
    assert "miniapps/miniapps.make" not in make
    assert "miniapp.link" not in hw
    assert "tools/miniapp-builder" not in hw + sim
    assert "dist/miniapps" not in hw
    assert "miniapps/packages/" not in hw
    assert "rm -rf simdisk/.rockbox/crazypod/miniapps/packages" in sim
    assert 'nargs="*"' in read("tools/crazypod_runtime_font_audit.py")
    workflow = read(".github/workflows/build.yml")
    assert "Set up Node.js" not in workflow
    assert "dist/miniapps/*.cpk" not in workflow
    assert not (ROOT / "miniapps/miniapps.make").exists()
    assert not list((ROOT / "miniapps/apps").glob("**/*.*"))
    assert not list((ROOT / "miniapps/themes").glob("**/*.*"))
    assert "for content_directory in Music Podcasts; do" in hw
    assert ".rockbox Music Podcasts" in hw
    assert "$PACKAGE_DIR/.rockbox/crazypod/miniapps/packages" not in hw
    assert 'cp "miniapps/packages/' not in sim
    assert '"$miniapp_package_dir/"' not in sim
    assert len(list((ROOT / "assets/crazypod-icons/basic").glob("*.bmp"))) == 16


if __name__ == "__main__":
    main()
    print("Music-only Home and package contract passed")
