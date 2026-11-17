#ifndef CRAZYPOD_IPHONE_APP_ICONS_H
#define CRAZYPOD_IPHONE_APP_ICONS_H

#include "lvgl.h"

/* 35x35 RGB565+A8 versions of the four user-supplied PNG references.
 * Explicit icon IDs avoid confusing the Voice Memos app with Now Playing.
 * Decode is cached and is never performed in the Click Wheel hot path. */
#define CRAZYPOD_IPHONE_APP_ICON_SIZE 35

enum crazypod_iphone_app_icon_id {
    CRAZYPOD_IPHONE_ICON_VOICE_MEMOS = 0,
    CRAZYPOD_IPHONE_ICON_MUSIC,
    CRAZYPOD_IPHONE_ICON_PODCASTS,
    CRAZYPOD_IPHONE_ICON_SETTINGS,
    CRAZYPOD_IPHONE_ICON_COUNT
};

const lv_image_dsc_t *crazypod_iphone_app_icon(
    enum crazypod_iphone_app_icon_id icon);

#endif
