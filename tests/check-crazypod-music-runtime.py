#!/usr/bin/env python3
"""Music-only runtime must not poll retired nonaudio application services."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
runtime = (root / "apps/crazypod/ui/app/crazypod_runtime_services.c").read_text(
    encoding="utf-8"
)
tick = runtime.split("void crazypod_runtime_services_tick(", 1)[1].split(
    "int crazypod_runtime_services_prepare_wait(", 1
)[0]
for forbidden in (
    "crazypod_photos_runtime_service(",
    "crazypod_wallpaper_crop_runtime_service(",
    "crazypod_books_feature_service_reader(",
    "crazypod_organizer_feature_service(",
    "crazypod_route_actions_service_notes(",
    "crazypod_photos_ensure_catalog(",
    "crazypod_videos_ensure_catalog(",
    "crazypod_miniapp_alarm_tick(",
):
    assert forbidden not in tick, forbidden
for required in (
    "crazypod_music_set_scan_suspended(locked)",
    "crazypod_music_library_service(",
    "crazypod_artwork_set_lock_suspended(",
    "crazypod_state_tick();",
):
    assert required in runtime, required
theme = (root / "apps/crazypod/ui/features/now_playing/crazypod_now_playing_theme.c").read_text(
    encoding="utf-8"
)
for forbidden in (
    "crazypod_miniapps_prepare(", "crazypod_miniapps_event(",
    "crazypod_now_playing_themes_open_id(", "crazypod_miniapps_feature_",
    "crazypod_miniapps_is_open(", "THEME_CONFIG",
):
    assert forbidden not in theme, forbidden
assert "CRAZYPOD_ARTWORK_CACHE_SIZE" in theme
assert "return 1;" in theme  # only the native built-in playback screen

assert "crazypod_miniapps_feature_" not in runtime
assert "crazypod_miniapp_alarm_" not in runtime
assert "service_miniapp(" not in runtime
assert "crazypod_now_playing_theme_prepare();" in runtime

print("Music-only service isolation checks passed")
