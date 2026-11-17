#ifndef CRAZYPOD_PRODUCT_GEOMETRY_H
#define CRAZYPOD_PRODUCT_GEOMETRY_H

/* Physical 320 x 240 iPod 6G screen: shared by the cached wallpaper
 * compositor and LVGL shell. Never let their frosted regions drift. */
#define CRAZYPOD_PRODUCT_SCREEN_WIDTH         320
#define CRAZYPOD_PRODUCT_SCREEN_HEIGHT        240

#define CRAZYPOD_PRODUCT_HOME_ROW_X           13
#define CRAZYPOD_PRODUCT_HOME_ROW_TOP         65
#define CRAZYPOD_PRODUCT_HOME_ROW_HEIGHT      25
#define CRAZYPOD_PRODUCT_HOME_ROW_GAP         2
#define CRAZYPOD_PRODUCT_HOME_ROW_COUNT       4

/* 2025-style vertical app reel: selected card floats in the center,
 * previous and next cards peek out above/below a clipped viewport.
 * No horizontal icon paging and no touch/OS scrolling. */
#define CRAZYPOD_PRODUCT_HOME_VIEWPORT_Y       49
#define CRAZYPOD_PRODUCT_HOME_VIEWPORT_HEIGHT  122
#define CRAZYPOD_PRODUCT_HOME_CARD_STEP        53
#define CRAZYPOD_PRODUCT_HOME_TRACK_PADDING    44
#define CRAZYPOD_PRODUCT_HOME_FOCUSED_Y        87
#define CRAZYPOD_PRODUCT_HOME_FOCUSED_HEIGHT   46
#define CRAZYPOD_PRODUCT_HOME_IDLE_HEIGHT      39

#define CRAZYPOD_PRODUCT_MINIPLAYER_MARGIN    12
#define CRAZYPOD_PRODUCT_MINIPLAYER_Y         181
#define CRAZYPOD_PRODUCT_MINIPLAYER_BOTTOM     8

#endif
