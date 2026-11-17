#!/usr/bin/env python3
"""Build local-only RGB565A8 / A8 icons. Never download or commit the artwork.

Source directory layout:
  home/music.png, home/podcasts.png, home/settings.png,
  home/voice_memos.png (optional, not displayed as Now Playing)
  menu/now_playing.png, menu/album_flow.png, menu/brightness.png, ...
Menu images must be transparent-background monochrome glyph PNGs.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "apps/crazypod/ui/crazypod_menu_icon.h"
DEFAULT_OUTPUT = (
    ROOT / "apps/crazypod/ui/presentation/crazypod_private_icon_assets.inc"
)
APP_NAMES = (
    ("VOICE_MEMOS", "voice_memos"),
    ("MUSIC", "music"),
    ("PODCASTS", "podcasts"),
    ("SETTINGS", "settings"),
)
REQUIRED_HOME = {"music", "podcasts", "settings"}
# Music, Podcasts, Now Playing and Settings and the quick-controls popover.
# Retired routes' compatibility-only icon IDs are deliberately not required.
REQUIRED_MENU = {
    "music", "now_playing", "album_flow", "music_library", "playlist",
    "artist", "album", "song", "search", "queue", "podcast",
    "favorite", "shuffle", "repeat", "sound", "display", "playback",
    "power", "controls", "language", "equalizer", "bass", "treble",
    "balance", "brightness", "backlight", "charging", "display_sleep",
    "motion_off", "power_timer", "usb", "storage", "sleep_timer",
    "timer_boot", "reset_timer", "beep", "keyclick", "speaker",
    "repeat_clicks", "settings", "date", "time", "menu",
    "visibility", "move_up", "move_down", "lock",
}


def icon_enum_names() -> list[str]:
    match = re.search(
        r"enum crazypod_menu_icon\s*\{(.*?)\};",
        HEADER.read_text(encoding="utf-8"), re.S
    )
    if match is None:
        raise ValueError("Cannot read CrazyPod menu icon IDs")
    names = re.findall(r"CRAZYPOD_MENU_ICON_([A-Z0-9_]+)", match.group(1))
    if names[0] != "NONE" or names[-1] != "COUNT":
        raise ValueError("Unexpected menu icon ID list")
    return names[1:-1]


def emit_image(name: str, data: bytes, side: int, color_format: str,
               stride: int) -> str:
    numbers = [f"0x{value:02x}" for value in data]
    rows = [
        "    " + ", ".join(numbers[i:i + 14]) + ","
        for i in range(0, len(numbers), 14)
    ]
    return (
        f"static LV_ATTRIBUTE_MEM_ALIGN const uint8_t {name}_data[] = {{\n"
        + "\n".join(rows)
        + f"\n}};\nstatic const lv_image_dsc_t {name} = {{\n"
        + "    .header.magic = LV_IMAGE_HEADER_MAGIC,\n"
        + f"    .header.cf = {color_format},\n"
        + f"    .header.w = {side},\n"
        + f"    .header.h = {side},\n"
        + f"    .header.stride = {stride},\n"
        + f"    .data_size = sizeof({name}_data),\n"
        + f"    .data = {name}_data,\n"
        + "};\n"
    )


def rgba565a8(image, side: int) -> bytes:
    image = image.convert("RGBA").resize((side, side), resample=1)
    output = bytearray()
    alpha = bytearray()
    for red, green, blue, opacity in image.getdata():
        rgb = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3)
        output.extend((rgb & 255, rgb >> 8))
        alpha.append(opacity)
    return bytes(output + alpha)


def mono_a8(image, side: int) -> bytes:
    image = image.convert("RGBA").resize((side, side), resample=1)
    alpha = bytes(image.getchannel("A").getdata())
    if min(alpha) >= 250:
        raise ValueError(
            "Menu symbol PNG must have a transparent background, "
            "not a solid app icon or a screenshot"
        )
    if max(alpha) < 16:
        raise ValueError("Menu symbol is empty")
    return alpha


def generate(source: Path, destination: Path, allow_partial: bool) -> None:
    if not source.is_dir():
        raise ValueError(
            "Private icon directory is missing; no Apple images are "
            "downloaded by this build"
        )
    try:
        from PIL import Image
    except ImportError as error:
        raise ValueError(
            "Private icon conversion needs Pillow: python3 -m pip install Pillow"
        ) from error

    names = icon_enum_names()
    entries = [
        "/* GENERATED LOCALLY — PRIVATE ARTWORK; DO NOT COMMIT. */",
        "/* Made from user-provided PNGs. No network or Apple downloads. */",
        "",
    ]
    app_table = []
    menu_table = []
    found_home = set()
    found_menu = set()

    for enum_name, slug in APP_NAMES:
        filename = source / "home" / (slug + ".png")
        if not filename.is_file():
            continue
        with Image.open(filename) as image:
            raw = rgba565a8(image, 21)
        symbol = "private_app_" + slug
        entries.append(
            emit_image(symbol, raw, 21, "LV_COLOR_FORMAT_RGB565A8", 42)
        )
        app_table.append(
            f"    [CRAZYPOD_IPHONE_ICON_{enum_name}] = &{symbol},"
        )
        found_home.add(slug)

    for enum_name in names:
        slug = enum_name.lower()
        filename = source / "menu" / (slug + ".png")
        if not filename.is_file():
            continue
        with Image.open(filename) as image:
            raw = mono_a8(image, 14)
        symbol = "private_menu_" + slug
        entries.append(emit_image(symbol, raw, 14, "LV_COLOR_FORMAT_A8", 14))
        menu_table.append(
            f"    [CRAZYPOD_MENU_ICON_{enum_name}] = &{symbol},"
        )
        found_menu.add(slug)

    if not found_home and not found_menu:
        raise ValueError("No matching home/*.png or menu/*.png files found")

    missing_home = sorted(REQUIRED_HOME - found_home)
    missing_menu = sorted(REQUIRED_MENU - found_menu)
    if (missing_home or missing_menu) and not allow_partial:
        raise ValueError(
            "Private icon build is incomplete. Missing home: " +
            ", ".join(missing_home or ["none"]) +
            "; missing menu: " + ", ".join(missing_menu or ["none"]) +
            ". Set CRAZYPOD_PRIVATE_ICONS_ALLOW_PARTIAL=1 for a "
            "mixed-asset local preview only."
        )

    entries.extend([
        "static const lv_image_dsc_t *const",
        "crazypod_local_app_icons[CRAZYPOD_IPHONE_ICON_COUNT] = {",
        *app_table,
        "};",
        "static const lv_image_dsc_t *const",
        "crazypod_local_menu_icons[CRAZYPOD_MENU_ICON_COUNT] = {",
        *menu_table,
        "};",
        "",
    ])
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text("\n".join(entries), encoding="utf-8")
    print(
        f"CrazyPod private icons: {len(found_home)} app artwork, "
        f"{len(found_menu)} menu glyphs; "
        "private pixels compiled locally, never committed"
    )
    if missing_home or missing_menu:
        print(
            "CrazyPod private icons: partial preview — remaining symbols "
            "use CrazyPod's original non-Apple fallback"
        )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--allow-partial", action="store_true")
    args = parser.parse_args()
    # On any failure, never reuse a previous private binary include.
    args.output.unlink(missing_ok=True)
    try:
        generate(args.source, args.output, args.allow_partial)
    except (ValueError, OSError) as error:
        print(f"CrazyPod private icon build failed: {error}",
              file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
