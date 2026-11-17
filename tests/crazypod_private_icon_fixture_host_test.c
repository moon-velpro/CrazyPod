#include <assert.h>
#include <stdio.h>

#include "crazypod_menu_icon_assets.h"
#include "crazypod_private_icon_assets.h"
#include "crazypod_iphone_app_icons.h"

/* The include is a locally generated fixture in a temporary build folder;
 * not an Apple asset and never checked into source. */
int main(void)
{
    const lv_image_dsc_t *music =
        crazypod_iphone_app_icon(CRAZYPOD_IPHONE_ICON_MUSIC);
    const lv_image_dsc_t *brightness =
        crazypod_menu_icon_asset(CRAZYPOD_MENU_ICON_BRIGHTNESS);
    const lv_image_dsc_t *fallback =
        crazypod_menu_icon_asset(CRAZYPOD_MENU_ICON_SHUFFLE);

    assert(music != NULL && music->header.w == CRAZYPOD_IPHONE_APP_ICON_SIZE);
    assert(music->header.cf == LV_COLOR_FORMAT_RGB565A8);
    assert(music->data_size == 35u * 35 * 3);
    assert(music->data[0] == 0x42);
    assert(crazypod_iphone_app_icon(CRAZYPOD_IPHONE_ICON_SETTINGS) == NULL);

    assert(brightness != NULL && brightness->header.w == 14);
    assert(brightness->header.cf == LV_COLOR_FORMAT_A8);
    assert(brightness->data_size == 14u * 14);
    assert(brightness->data[0] == 0x99);
    assert(fallback != NULL && fallback != brightness);
    assert(crazypod_private_icon_menu(CRAZYPOD_MENU_ICON_NONE) == NULL);
    puts("CrazyPod locally linked private glyph override and public fallback passed");
    return 0;
}
