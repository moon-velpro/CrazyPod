#ifndef CRAZYPOD_PRIVATE_ICON_ASSETS_H
#define CRAZYPOD_PRIVATE_ICON_ASSETS_H

#include "lvgl.h"
#include "../crazypod_menu_icon.h"
#include "crazypod_iphone_app_icons.h"

/* A source-only interface. The private RGB565A8/A8 pixels are generated
 * locally at build time and ignored by Git. Public CI links empty stubs. */
const lv_image_dsc_t *crazypod_private_icon_app(
    enum crazypod_iphone_app_icon_id icon);
const lv_image_dsc_t *crazypod_private_icon_menu(
    enum crazypod_menu_icon icon);

#endif
