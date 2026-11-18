#!/usr/bin/env python3
"""Prevent the removed cartridge emulator from returning to firmware."""

from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[1]


def source(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def test_retired_emulator_absent() -> None:
    assert not (ROOT / "apps/crazypod/gameboy").exists()
    assert not (ROOT / "packaging/gameboy").exists()
    assert not (ROOT / "docs/CRAZYPOD_GAMEBOY.zh-CN.md").exists()
    assert not list(
        (ROOT / "apps/crazypod/ui/features/miniapps")
        .glob("crazypod_gameboy_*")
    )
    for path in (
        "apps/apps.make",
        "apps/SOURCES",
        "apps/crazypod/crazypod_apps.c",
        "apps/crazypod/ui/shell/crazypod_app_catalog.c",
        "apps/crazypod/ui/app/crazypod_app_launcher.c",
        "apps/crazypod/ui/features/miniapps/crazypod_miniapps_feature.c",
        "apps/crazypod/ui/features/miniapps/crazypod_miniapps_feature.h",
        "apps/crazypod/ui/navigation/crazypod_route_registry.c",
        "build-hw.sh",
        "build-sim.sh",
    ):
        text = source(path)
        assert not re.search(
            r"gameboy|GAMEBOY|GB / GBC|MiniApps/Games/GB", text
        ), path


def test_legacy_state_and_home_icons() -> None:
    header = source("apps/crazypod/crazypod_apps.h")
    state = source("apps/crazypod/crazypod_state.c")
    icons = source("apps/crazypod/crazypod_icons.h")
    desktop = source("apps/crazypod/ui/shell/crazypod_desktop.c")

    assert "#define CRAZYPOD_APP_COUNT 16" in header
    assert "#define CRAZYPOD_APP_DISK_COUNT 17" in header
    assert "menu_order[CRAZYPOD_APP_DISK_COUNT]" in state
    assert "state.menu_count <= CRAZYPOD_APP_DISK_COUNT" in state
    assert "#define CRAZYPOD_ICON_COUNT 16" in icons
    assert "crazypod_app_catalog_index(app->id)" in desktop
    assert "icon_descriptors[app->id]" not in desktop

    basic = ROOT / "assets/crazypod-icons/basic"
    assert len(list(basic.glob("*.bmp"))) == 16


if __name__ == "__main__":
    test_retired_emulator_absent()
    test_legacy_state_and_home_icons()
    print("Retired emulator and Home icon contract checks passed")
