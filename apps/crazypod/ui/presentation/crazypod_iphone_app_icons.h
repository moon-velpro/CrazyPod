#ifndef CRAZYPOD_IPHONE_APP_ICONS_H
#define CRAZYPOD_IPHONE_APP_ICONS_H

#include "lvgl.h"
#include "../shell/crazypod_home_model.h"

/* Uses the four user-supplied app icon images, downsampled to 21x21
 * RGB565 + A8 with the original transparent rounded corners.
 * No filesystem access, scaling, or image decoding in the wheel hot path. */
#define CRAZYPOD_IPHONE_APP_ICON_SIZE 21

const lv_image_dsc_t *crazypod_iphone_app_icon(
    enum crazypod_home_destination destination);

#endif
