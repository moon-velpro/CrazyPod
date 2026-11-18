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
ui_root = (root.parent / "crazypod_ui.c").read_text(encoding="utf-8")
native = source("shell/crazypod_desktop_native.c")
home_actions = source("shell/crazypod_home_actions.c")
app_input = source("app/crazypod_app_input.c")
registry = source("navigation/crazypod_route_registry.c")

assert "HOME_VISIBLE_ROWS" in home, "home must remain a vertical wheel menu"
project_root = root.parents[2]
customize = source("features/customize/crazypod_customize_catalog.c")
native_header = source("shell/crazypod_desktop_native.h")
sources = (project_root / "apps/SOURCES").read_text(encoding="utf-8")
build_hw = (project_root / "build-hw.sh").read_text(encoding="utf-8")
build_sim = (project_root / "build-sim.sh").read_text(encoding="utf-8")
assert not (project_root / "assets/crazypod-icons").exists(), (
    "unused 16-theme bitmaps must not ship in the repo"
)
assert "16 complete icon themes" not in customize
assert 'CP_TR("Icons")' not in customize
assert 'CP_TR("Icon Size")' not in customize
assert "scaled_icons" not in native, "old native icon cache must be retired"
assert "native_render(" not in native_header
assert "crazypod/crazypod_icons.c" not in sources
assert "assets/crazypod-icons" not in build_hw + build_sim
assert "home_symbols" in home, "Home app glyphs must be a fixed set"
assert "render_desktop_icons(" not in home, "old Android-like icon carousel returned"
assert "crazypod_desktop_native_invalidate(" not in home, (
    "LVGL Home must not schedule an abandoned native icon redraw"
)
assert "HOME_SPRING_STIFFNESS" not in home, (
    "home cannot spend cycles animating an invisible carousel"
)
assert "crazypod_present_set_home_interaction(false);" in ui_root, (
    "wheel touch must not freeze LVGL Home selection updates"
)
assert "crazypod_present_queue_full();" in native.split(
    "void crazypod_desktop_native_restore_after_modal(void)", 1
)[1].split("lv_obj_t *crazypod_desktop_native_create_modal_underlay", 1)[0], (
    "Home modal dismissal must request a complete framebuffer repaint"
)
assert "crazypod_wallpaper_prepare_frosted_capsule(" not in capsule, (
    "home mini-player must not require frosted framebuffer rendering"
)
assert "crazypod_popup_centered_geometry" not in home_actions, (
    "Home quick actions must be proper pages instead of popups"
)
assert "crazypod_home_actions_show()" not in app_input, (
    "short MENU must not open the old Home modal"
)
assert "crazypod_app_launcher_open_root(HOME_ROUTE_ACTIONS)" in app_input, (
    "short MENU should open a genuine Home action route"
)
for name in ("HOME_ROUTE_ACTIONS", "HOME_ROUTE_BRIGHTNESS",
             "HOME_ROUTE_VOLUME"):
    assert name in routes and ("SHELL_ROUTE(" + name + ")") in registry, (
        "Home control page is missing from route stack registration"
    )
assert "HOME_ROUTE_BRIGHTNESS" in actions and "HOME_ROUTE_VOLUME" in actions, (
    "brightness and volume must be actual child routes"
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
