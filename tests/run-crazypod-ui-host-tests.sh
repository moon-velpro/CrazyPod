#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
test_root=$(mktemp -d)
trap 'rm -rf "$test_root"' EXIT HUP INT TERM

cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/apps/crazypod" \
    -I"$repo_root/apps/crazypod/ui" \
    "$repo_root/apps/crazypod/crazypod_collation.c" \
    "$repo_root/apps/crazypod/ui/features/organizer/crazypod_calendar_model.c" \
    "$repo_root/apps/crazypod/ui/presentation/crazypod_ui_menu_layout.c" \
    "$repo_root/apps/crazypod/ui/presentation/crazypod_scene_motion.c" \
    "$repo_root/apps/crazypod/ui/presentation/crazypod_ui_text.c" \
    "$repo_root/apps/crazypod/ui/navigation/crazypod_alpha_jump.c" \
    "$repo_root/apps/crazypod/ui/navigation/crazypod_feature_dispatcher.c" \
    "$repo_root/apps/crazypod/ui/navigation/crazypod_navigation_command.c" \
    "$repo_root/apps/crazypod/ui/navigation/crazypod_route_registry.c" \
    "$repo_root/tests/crazypod_ui_pure_host_test.c" \
    -o "$test_root/crazypod_ui_pure_host_test"

"$test_root/crazypod_ui_pure_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/tests/crazypod-books-stubs" \
    -I"$repo_root/apps/crazypod" \
    "$repo_root/apps/crazypod/crazypod_books.c" \
    "$repo_root/tests/crazypod_books_catalog_host_test.c" \
    -o "$test_root/crazypod_books_catalog_host_test"

"$test_root/crazypod_books_catalog_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/tests/crazypod-frameclock-stubs" \
    -I"$repo_root/apps/crazypod" \
    -I"$repo_root/apps/crazypod/ui" \
    "$repo_root/apps/crazypod/crazypod_apps.c" \
    "$repo_root/apps/crazypod/ui/features/settings/crazypod_settings_controller.c" \
    "$repo_root/tests/crazypod_settings_reorder_host_test.c" \
    -o "$test_root/crazypod_settings_reorder_host_test"

"$test_root/crazypod_settings_reorder_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/tests/crazypod-lyrics-stubs" \
    -I"$repo_root/apps/crazypod" \
    "$repo_root/apps/crazypod/crazypod_lyrics.c" \
    "$repo_root/tests/crazypod_lyrics_host_test.c" \
    -o "$test_root/crazypod_lyrics_host_test"

"$test_root/crazypod_lyrics_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/tests/crazypod-notes-stubs" \
    -I"$repo_root/apps/crazypod" \
    "$repo_root/apps/crazypod/crazypod_notes.c" \
    "$repo_root/tests/crazypod_notes_host_test.c" \
    -o "$test_root/crazypod_notes_host_test"

"$test_root/crazypod_notes_host_test" "$test_root/notes"

cc -std=c99 -Wall -Wextra -Werror \
    "$repo_root/tests/crazypod_liquid_music_host_test.c" \
    -o "$test_root/crazypod_liquid_music_host_test"

"$test_root/crazypod_liquid_music_host_test"

# Video player is no longer linked. Keep the compatibility stubs inert.
cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/tests/crazypod-video-stubs" \
    -I"$repo_root/apps/crazypod" \
    "$repo_root/apps/crazypod/crazypod_videos_disabled.c" \
    "$repo_root/tests/crazypod_video_disabled_host_test.c" \
    -o "$test_root/crazypod_video_disabled_host_test"

"$test_root/crazypod_video_disabled_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    "$repo_root/tests/crazypod_product_scope_host_test.c" \
    -o "$test_root/crazypod_product_scope_host_test"

"$test_root/crazypod_product_scope_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    "$repo_root/tests/crazypod_music_root_menu_host_test.c" \
    -o "$test_root/crazypod_music_root_menu_host_test"

"$test_root/crazypod_music_root_menu_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    "$repo_root/tests/crazypod_home_model_host_test.c" \
    -o "$test_root/crazypod_home_model_host_test"

"$test_root/crazypod_home_model_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/tests/crazypod-home-input-stubs" \
    -I"$repo_root/apps/crazypod/ui" \
    "$repo_root/apps/crazypod/ui/navigation/crazypod_input_event.c" \
    "$repo_root/apps/crazypod/ui/shell/crazypod_home_input.c" \
    "$repo_root/tests/crazypod_home_input_host_test.c" \
    -o "$test_root/crazypod_home_input_host_test"

"$test_root/crazypod_home_input_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/tests/crazypod-home-input-stubs" \
    -I"$repo_root/apps/crazypod/ui" \
    "$repo_root/apps/crazypod/ui/navigation/crazypod_remote_multitap.c" \
    "$repo_root/tests/crazypod_remote_multitap_host_test.c" \
    -o "$test_root/crazypod_remote_multitap_host_test"

"$test_root/crazypod_remote_multitap_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/tests/crazypod-iap-stubs" \
    -I"$repo_root/firmware/export" \
    -I"$repo_root/apps/crazypod/accessory" \
    "$repo_root/apps/crazypod/accessory/crazypod_iap_simple.c" \
    "$repo_root/tests/crazypod_iap_simple_host_test.c" \
    -o "$test_root/crazypod_iap_simple_host_test"

"$test_root/crazypod_iap_simple_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/tests/crazypod-home-input-stubs" \
    -I"$repo_root/apps/crazypod/ui" \
    "$repo_root/apps/crazypod/ui/navigation/crazypod_input_event.c" \
    "$repo_root/apps/crazypod/ui/features/books/crazypod_book_reader_input.c" \
    "$repo_root/tests/crazypod_book_reader_input_host_test.c" \
    -o "$test_root/crazypod_book_reader_input_host_test"

"$test_root/crazypod_book_reader_input_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/apps/crazypod/ui" \
    "$repo_root/apps/crazypod/ui/app/crazypod_screen_off_policy.c" \
    "$repo_root/tests/crazypod_screen_off_policy_host_test.c" \
    -o "$test_root/crazypod_screen_off_policy_host_test"

"$test_root/crazypod_screen_off_policy_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/apps/crazypod" \
    "$repo_root/tests/crazypod_screen_recording_policy_host_test.c" \
    -o "$test_root/crazypod_screen_recording_policy_host_test"

"$test_root/crazypod_screen_recording_policy_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/apps/crazypod" \
    "$repo_root/apps/crazypod/crazypod_audio_memory_policy.c" \
    "$repo_root/tests/crazypod_audio_memory_policy_host_test.c" \
    -o "$test_root/crazypod_audio_memory_policy_host_test"

"$test_root/crazypod_audio_memory_policy_host_test"

cc -std=gnu99 -Wall -Wextra -Werror \
    -I"$repo_root/tests/crazypod-buflib-stubs" \
    -I"$repo_root/firmware/include" \
    -I"$repo_root/apps/crazypod" \
    "$repo_root/firmware/buflib_mempool.c" \
    "$repo_root/apps/crazypod/crazypod_audio_memory_policy.c" \
    "$repo_root/tests/crazypod_audio_memory_buflib_host_test.c" \
    -o "$test_root/crazypod_audio_memory_buflib_host_test"

"$test_root/crazypod_audio_memory_buflib_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/tests/crazypod-image-stubs" \
    -I"$repo_root/apps/crazypod" \
    "$repo_root/apps/crazypod/crazypod_image.c" \
    "$repo_root/tests/crazypod_image_host_test.c" \
    -o "$test_root/crazypod_image_host_test"

# Compile the shipping A8 icon rasterizer on Linux as well as on ARM.
cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/tests/crazypod-image-stubs" \
    -I"$repo_root/apps/crazypod/ui" \
    -I"$repo_root/apps/crazypod/ui/presentation" \
    "$repo_root/apps/crazypod/ui/presentation/crazypod_menu_icon_assets.c" \
    "$repo_root/apps/crazypod/ui/presentation/crazypod_private_icon_assets.c" \
    "$repo_root/tests/crazypod_apple_glyph_host_test.c" \
    -o "$test_root/crazypod_apple_glyph_host_test"

# Public builds must not include embedded Apple/user-supplied bitmap data.
# Explicit local-only overrides have a separate generated source path.
cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/tests/crazypod-image-stubs" \
    -I"$repo_root/apps/crazypod/ui" \
    -I"$repo_root/apps/crazypod/ui/presentation" \
    "$repo_root/apps/crazypod/ui/presentation/crazypod_iphone_app_icons.c" \
    "$repo_root/apps/crazypod/ui/presentation/crazypod_private_icon_assets.c" \
    "$repo_root/tests/crazypod_iphone_app_icons_host_test.c" \
    -o "$test_root/crazypod_iphone_app_icons_host_test"

"$test_root/crazypod_iphone_app_icons_host_test"

# Compile a synthetic private icon layer, without downloading or checking
# in Apple artwork. Verify public fallback and private A8/RGB565A8 links.
cat > "$test_root/crazypod_private_icon_fixture.inc" <<'END_PRIVATE_ICON_FIXTURE'
static const uint8_t fixture_app[21 * 21 * 3] = { 0x42 };
static const lv_image_dsc_t fixture_app_image = {
    .header.magic = LV_IMAGE_HEADER_MAGIC,
    .header.cf = LV_COLOR_FORMAT_RGB565A8,
    .header.w = 21,
    .header.h = 21,
    .header.stride = 42,
    .data_size = sizeof(fixture_app),
    .data = fixture_app,
};
static const uint8_t fixture_menu[14 * 14] = { 0x99 };
static const lv_image_dsc_t fixture_menu_image = {
    .header.magic = LV_IMAGE_HEADER_MAGIC,
    .header.cf = LV_COLOR_FORMAT_A8,
    .header.w = 14,
    .header.h = 14,
    .header.stride = 14,
    .data_size = sizeof(fixture_menu),
    .data = fixture_menu,
};
static const lv_image_dsc_t *const
    crazypod_local_app_icons[CRAZYPOD_IPHONE_ICON_COUNT] = {
    [CRAZYPOD_IPHONE_ICON_MUSIC] = &fixture_app_image,
};
static const lv_image_dsc_t *const
    crazypod_local_menu_icons[CRAZYPOD_MENU_ICON_COUNT] = {
    [CRAZYPOD_MENU_ICON_BRIGHTNESS] = &fixture_menu_image,
};
END_PRIVATE_ICON_FIXTURE
cc -std=c99 -Wall -Wextra -Werror \
    -DCRAZYPOD_PRIVATE_ICONS -DCRAZYPOD_PRIVATE_ICON_TEST_FIXTURE \
    -I"$test_root" \
    -I"$repo_root/tests/crazypod-image-stubs" \
    -I"$repo_root/apps/crazypod/ui" \
    -I"$repo_root/apps/crazypod/ui/presentation" \
    "$repo_root/apps/crazypod/ui/presentation/crazypod_private_icon_assets.c" \
    "$repo_root/apps/crazypod/ui/presentation/crazypod_menu_icon_assets.c" \
    "$repo_root/apps/crazypod/ui/presentation/crazypod_iphone_app_icons.c" \
    "$repo_root/tests/crazypod_private_icon_fixture_host_test.c" \
    -o "$test_root/crazypod_private_icon_fixture_host_test"
"$test_root/crazypod_private_icon_fixture_host_test"

"$test_root/crazypod_apple_glyph_host_test"

"$test_root/crazypod_image_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/tests/crazypod-playlist-stubs" \
    -I"$repo_root/apps/crazypod" \
    "$repo_root/tests/crazypod_core_alloc_host_stub.c" \
    "$repo_root/apps/crazypod/crazypod_playlist.c" \
    "$repo_root/tests/crazypod_playlist_host_test.c" \
    -o "$test_root/crazypod_playlist_host_test"

"$test_root/crazypod_playlist_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/tests/crazypod-playlist-stubs" \
    -I"$repo_root/apps/crazypod" \
    "$repo_root/tests/crazypod_core_alloc_host_stub.c" \
    "$repo_root/apps/crazypod/crazypod_music_storage.c" \
    "$repo_root/tests/crazypod_music_storage_host_test.c" \
    -o "$test_root/crazypod_music_storage_host_test"

"$test_root/crazypod_music_storage_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    -DMAX_PATH=1024 \
    -I"$repo_root/tests/miniapp-host-stubs" \
    -I"$repo_root/apps/crazypod" \
    "$repo_root/tests/crazypod_music_validation_host_test.c" \
    -o "$test_root/crazypod_music_validation_host_test"

"$test_root/crazypod_music_validation_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/tests/crazypod-frameclock-stubs" \
    -I"$repo_root/apps/crazypod" \
    "$repo_root/apps/crazypod/crazypod_frameclock.c" \
    "$repo_root/tests/crazypod_frameclock_host_test.c" \
    -o "$test_root/crazypod_frameclock_host_test"

"$test_root/crazypod_frameclock_host_test"

cc -std=c99 -Wall -Wextra -Werror \
    -I"$repo_root/tests/crazypod-font-stubs" \
    "$repo_root/tests/crazypod_runtime_font_host_test.c" \
    -o "$test_root/crazypod_runtime_font_host_test"

"$test_root/crazypod_runtime_font_host_test"

python3 "$repo_root/tests/test-crazypod-menu-icons.py"
python3 -m py_compile "$repo_root/tools/prepare-crazypod-private-icons.py"
python3 "$repo_root/tests/test-crazypod-photo-cache-policy.py"
