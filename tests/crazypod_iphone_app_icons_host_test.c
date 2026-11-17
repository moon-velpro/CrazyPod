#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "crazypod_iphone_app_icons.h"

/* Pixel signatures computed directly from the four user-uploaded 512px
 * RGBA PNGs resized once at 21px using Pillow/Lanczos to RGB565A8.
 * These protect against accidental palette changes and base64 corruption. */
static const uint32_t expected_crc[CRAZYPOD_IPHONE_ICON_COUNT] = {
    0x4e050b06u, /* Upload 4 -> Voice Memos */
    0xaef615a4u, /* Upload 1 -> Music */
    0xdc16fb6eu, /* Upload 2 -> Podcasts */
    0xf28f92eeu, /* Upload 3 -> Settings */
};
static const uint16_t expected_middle[CRAZYPOD_IPHONE_ICON_COUNT] = {
    0xf63au, 0xf79eu, 0xff5fu, 0x0000u
};

static uint32_t crc32_bytes(const uint8_t *pixels, unsigned size)
{
    uint32_t crc = 0xffffffffu;
    unsigned i;

    for(i = 0; i < size; ++i) {
        int b;

        crc ^= pixels[i];
        for(b = 0; b < 8; ++b)
            crc = (crc >> 1) ^
                ((crc & 1) ? 0xedb88320u : 0u);
    }
    return ~crc;
}

int main(void)
{
    int i;

    assert(crazypod_iphone_app_icon(
        (enum crazypod_iphone_app_icon_id)-1) == NULL);
    assert(crazypod_iphone_app_icon(
        CRAZYPOD_IPHONE_ICON_COUNT) == NULL);
    for(i = 0; i < CRAZYPOD_IPHONE_ICON_COUNT; ++i) {
        const lv_image_dsc_t *icon = crazypod_iphone_app_icon(
            (enum crazypod_iphone_app_icon_id)i);
        const uint8_t *pixels;
        unsigned center = ((10 * 21) + 10) * 2;

        assert(icon != NULL);
        assert(icon == crazypod_iphone_app_icon(
            (enum crazypod_iphone_app_icon_id)i));
        assert(icon->header.magic == LV_IMAGE_HEADER_MAGIC);
        assert(icon->header.cf == LV_COLOR_FORMAT_RGB565A8);
        assert(icon->header.w == 21);
        assert(icon->header.h == 21);
        assert(icon->header.stride == 42);
        assert(icon->data_size == 21u * 21 * 3);
        pixels = icon->data;
        assert(pixels != NULL);
        assert(crc32_bytes(pixels, icon->data_size) ==
               expected_crc[i]);
        assert((uint16_t)(pixels[center] |
                          (pixels[center + 1] << 8)) ==
               expected_middle[i]);
        assert(pixels[21 * 21 * 2] < 8); /* transparent corner */
        assert(pixels[21 * 21 * 2 + 10 * 21 + 10] == 255);
    }
    puts("CrazyPod user-provided iPhone app icons passed");
    return 0;
}
