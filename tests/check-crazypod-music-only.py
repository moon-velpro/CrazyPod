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

    order = apps.split("static const uint8_t default_order", 1)[1].split("};", 1)[0]
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
    for retired in (
        "APP_MINI_APPS", "APP_PHOTOS", "APP_BOOKS", "APP_NOTES",
        "APP_CONTACTS", "APP_CALENDAR", "APP_WORKOUTS", "APP_STOPWATCH",
        "APP_EXTRAS", "APP_CUSTOMIZE",
    ):
        assert "case CRAZYPOD_" + retired not in launcher, retired
    for script in ("build-hw.sh", "build-sim.sh"):
        content = read(script)
        assert "for icon in music podcasts shuffle screen_lock settings; do" in content
        assert "crazypod/icons/basic" in content
    hw = read("build-hw.sh")
    sim = read("build-sim.sh")
    assert "for content_directory in Music Podcasts; do" in hw
    assert ".rockbox Music Podcasts" in hw
    assert "$PACKAGE_DIR/.rockbox/crazypod/miniapps/packages" not in hw
    assert 'cp "miniapps/packages/' not in sim
    assert '"$miniapp_package_dir/"' not in sim
    assert len(list((ROOT / "assets/crazypod-icons/basic").glob("*.bmp"))) == 16


if __name__ == "__main__":
    main()
    print("Music-only Home and package contract passed")
