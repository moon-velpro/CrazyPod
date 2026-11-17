#include "config.h"

#ifdef IPOD_6G

#include <string.h>

#include "lcd.h"
#include "system.h"
#include "lvgl.h"
#include "src/misc/cache/instance/lv_image_cache.h"

#include "../../crazypod_frameclock.h"
#include "../../crazypod_image.h"
#include "../../platform/crazypod_platform_display.h"
#include "crazypod_desktop_native.h"

#define NATIVE_TOP CRAZYPOD_DESKTOP_NATIVE_TOP
#define NATIVE_BOTTOM CRAZYPOD_DESKTOP_NATIVE_BOTTOM
#define NATIVE_HEIGHT (NATIVE_BOTTOM - NATIVE_TOP)

/* The 17-slot 120px RGBA carousel and its framebuffer backdrop are retired.
 * The actual vertical Home is entirely LVGL-owned. Retain only the existing
 * 320x103 RGB565 underlay used by Hold, quick controls and Now Playing. */
static fb_data modal_underlay[LCD_WIDTH * NATIVE_HEIGHT]
    CACHEALIGN_AT_LEAST_ATTR(16);
static lv_image_dsc_t modal_underlay_descriptor;
static bool preserve_modal_underlay;

void crazypod_desktop_native_reset(void)
{
    preserve_modal_underlay = false;
}

void crazypod_desktop_native_invalidate(bool discard_backdrop)
{
    (void)discard_backdrop;
}

void crazypod_desktop_native_invalidate_icons(void)
{
}

void crazypod_desktop_native_prepare_modal(void)
{
    crazypod_present_take_fullscreen_ownership();
    crazypod_present_queue_full();
}

void crazypod_desktop_native_restore_after_modal(void)
{
    /* LVGL invalidates the removed overlay. The retired native icon
     * carousel no longer needs its own delayed fullscreen submission. */
}

lv_obj_t *crazypod_desktop_native_create_modal_underlay(
    lv_obj_t *parent)
{
    const fb_data *framebuffer =
        (const fb_data *)crazypod_platform_display_framebuffer();
    lv_obj_t *image;

    if(parent == NULL || framebuffer == NULL)
        return NULL;
    if(!preserve_modal_underlay) {
        if(modal_underlay_descriptor.header.magic ==
           LV_IMAGE_HEADER_MAGIC)
            lv_image_cache_drop(&modal_underlay_descriptor);
        memcpy(
            modal_underlay,
            framebuffer + NATIVE_TOP * LCD_WIDTH,
            sizeof(modal_underlay));
        if(!crazypod_image_configure_rgb565(
               &modal_underlay_descriptor, modal_underlay,
               LCD_WIDTH, NATIVE_HEIGHT))
            return NULL;
    }
    preserve_modal_underlay = false;
    if(modal_underlay_descriptor.header.magic != LV_IMAGE_HEADER_MAGIC)
        return NULL;
    image = lv_image_create(parent);
    lv_image_set_src(image, &modal_underlay_descriptor);
    lv_obj_set_pos(image, 0, NATIVE_TOP);
    lv_obj_remove_flag(image, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_move_background(image);
    return image;
}

void crazypod_desktop_native_preserve_modal_underlay(void)
{
    preserve_modal_underlay =
        modal_underlay_descriptor.header.magic == LV_IMAGE_HEADER_MAGIC;
}

void crazypod_desktop_native_capture_flush(const lv_area_t *area)
{
    /* The obsolete manual framebuffer icon overlay has no back buffer to
     * synchronize. LVGL owns the vertical Home and flush scheduling. */
    (void)area;
}

bool crazypod_desktop_native_render(
    const int *app_indices, const int *centers_x, int icon_count,
    int icon_size, bool blocked)
{
    (void)app_indices;
    (void)centers_x;
    (void)icon_count;
    (void)icon_size;
    (void)blocked;
    return false;
}

bool crazypod_desktop_native_render_snapshot(
    const int *app_indices, const int *centers_x, int icon_count,
    int icon_size)
{
    (void)app_indices;
    (void)centers_x;
    (void)icon_count;
    (void)icon_size;
    return false;
}

#endif
