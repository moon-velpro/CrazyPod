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
