/* Exercise the actual C rasterizer on the host, not just source strings. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "crazypod_menu_icon_assets.h"

#define GLYPH_PIXELS (14 * 14)

static void check_icon(enum crazypod_menu_icon id)
{
    const lv_image_dsc_t *image = crazypod_menu_icon_asset(id);
    const lv_image_dsc_t *again = crazypod_menu_icon_asset(id);
    unsigned occupied = 0;
    uint8_t peak = 0;
    unsigned j;

    assert(image != NULL);
    assert(again == image);
    assert(image->header.magic == LV_IMAGE_HEADER_MAGIC);
    assert(image->header.cf == LV_COLOR_FORMAT_A8);
    assert(image->header.w == 14);
    assert(image->header.h == 14);
    assert(image->header.stride == 14);
    assert(image->data_size == GLYPH_PIXELS);
    assert(image->data != NULL);
    for(j = 0; j < GLYPH_PIXELS; ++j) {
        uint8_t value = image->data[j];

        if(value != 0)
            ++occupied;
        if(value > peak)
            peak = value;
    }
    assert(occupied >= 10);
    assert(peak >= 160);
}

int main(void)
{
    int i;
    const lv_image_dsc_t *podcast;
    const lv_image_dsc_t *volume;
    const lv_image_dsc_t *music;
    const lv_image_dsc_t *settings;

    assert(crazypod_menu_icon_asset(CRAZYPOD_MENU_ICON_NONE) == NULL);
    assert(crazypod_menu_icon_asset(CRAZYPOD_MENU_ICON_COUNT) == NULL);
    for(i = CRAZYPOD_MENU_ICON_NONE + 1;
        i < CRAZYPOD_MENU_ICON_COUNT; ++i)
        check_icon((enum crazypod_menu_icon)i);

    podcast = crazypod_menu_icon_asset(CRAZYPOD_MENU_ICON_PODCAST);
    volume = crazypod_menu_icon_asset(CRAZYPOD_MENU_ICON_SPEAKER);
    music = crazypod_menu_icon_asset(CRAZYPOD_MENU_ICON_MUSIC);
    settings = crazypod_menu_icon_asset(CRAZYPOD_MENU_ICON_SETTINGS);
    assert(podcast != volume);
    assert(music != podcast);
    assert(settings != music);
    assert(memcmp(podcast->data, volume->data, GLYPH_PIXELS) != 0);
    assert(memcmp(podcast->data, music->data, GLYPH_PIXELS) != 0);
    puts("CrazyPod original iPod-style A8 glyph rasterizer passed");
    return 0;
}
