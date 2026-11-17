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
