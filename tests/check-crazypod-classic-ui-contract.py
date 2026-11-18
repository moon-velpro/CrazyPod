#!/usr/bin/env python3
"""Guard the non-Android interaction contract of the Classic UI shell."""
from pathlib import Path

root = Path(__file__).resolve().parents[1] / "apps" / "crazypod" / "ui"


def source(relative):
    return (root / relative).read_text(encoding="utf-8")


home = source("shell/crazypod_desktop.c")
capsule = source("shell/crazypod_now_capsule.c")
routes = source("navigation/crazypod_ui_routes.h")
actions = source("app/crazypod_route_actions.c")
renderer = source("app/crazypod_route_renderer.c")
choices = source("presentation/crazypod_choice_overlay.c")
now_actions = source("features/now_playing/crazypod_now_playing_overlay.c")

assert "HOME_VISIBLE_ROWS" in home, "home must remain a vertical wheel menu"
assert "render_desktop_icons(" not in home, "old Android-like icon carousel returned"
assert "crazypod_wallpaper_prepare_frosted_capsule(" not in capsule, (
    "home mini-player must not require frosted framebuffer rendering"
)
assert "SETTINGS_ROUTE_VALUE_CHOICES" in routes, (
    "settings choices must be proper navigation routes"
)
assert "SETTINGS_ROUTE_VALUE_CHOICES, item, selected" in actions, (
    "settings choices cannot fall back to modal overlays"
)
assert "crazypod_menu_preview_render(state, false)" not in renderer, (
    "generic lists must not return to split-pane app previews"
)
assert "crazypod_popup_centered_geometry(" not in choices, (
    "choice pages may not use centered popup geometry"
)
assert "crazypod_popup_centered_geometry(" not in now_actions, (
    "Now Playing actions must be navigation pages, not dialogs"
)
print("Classic 2025 shell UI contract passed")
