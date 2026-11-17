#include <assert.h>

#include "../apps/crazypod/ui/presentation/crazypod_liquid_music.h"
#include "../apps/crazypod/crazypod_product_geometry.h"

/* Simulates the exact RGB channel shade model used by the double-buffered
 * Now Playing background presentation. The most luminous RGB888 backdrop
 * must still leave high-contrast white text legible. */
static unsigned shade_channel(unsigned foreground, unsigned background,
                               unsigned alpha)
{
    return (foreground * (255u - alpha) +
            background * alpha + 127u) / 255u;
}

int main(void)
{
    /* Artwork is always larger than the former 108 px miniature. */
    assert(CRAZYPOD_LM_ARTWORK_SIZE == 128);
    assert(CRAZYPOD_LM_ARTWORK_X >= 0);
    assert(CRAZYPOD_LM_ARTWORK_Y >= 32);
    assert(CRAZYPOD_LM_ARTWORK_X + CRAZYPOD_LM_ARTWORK_SIZE + 12 <=
           CRAZYPOD_LM_INFO_X);
    assert(CRAZYPOD_LM_INFO_X + CRAZYPOD_LM_INFO_WIDTH <= 320);
    assert(CRAZYPOD_LM_ARTWORK_Y + CRAZYPOD_LM_ARTWORK_SIZE + 8 <=
           CRAZYPOD_LM_WAVE_Y);
    assert(CRAZYPOD_LM_WAVE_X + CRAZYPOD_LM_WAVE_WIDTH <= 320);
    assert(CRAZYPOD_LM_WAVE_Y + CRAZYPOD_LM_WAVE_HEIGHT <= 218);

    /* 255 against the darkest overlay must not wash out white metadata. */
    assert(CRAZYPOD_LM_SHADE_OPA >= 150);
    assert(CRAZYPOD_LM_SHADE_OPA <= 255);
    assert(shade_channel(255, 5, CRAZYPOD_LM_SHADE_OPA) <= 111);
    assert(shade_channel(255, 8, CRAZYPOD_LM_SHADE_OPA) <= 111);

    /* Apple-like root lists use one soft highlight on light paper;
     * the background does not compete with Music artwork. */
    assert(CRAZYPOD_LM_NATIVE_BG != CRAZYPOD_LM_CANVAS);
    assert(CRAZYPOD_LM_NATIVE_BG > 0xE8E8E8u);
    assert(CRAZYPOD_LM_NATIVE_INK < 0x444444u);
    assert(CRAZYPOD_LM_NATIVE_SELECTED != CRAZYPOD_LM_NATIVE_BG);
    assert(CRAZYPOD_LM_HOME_IDLE_OPA == 0);
    assert(CRAZYPOD_LM_WAVE_HEIGHT <= 10);
    assert(CRAZYPOD_LM_WAVE_Y + CRAZYPOD_LM_WAVE_HEIGHT < 200);

    /* The real album's dominant hue, not the file path or arbitrary theme,
     * changes the tint behind its cached blurred cover. Keep channels
     * bounded so contrast selection still has usable dark/light choices. */
    {
        unsigned red = crazypod_lm_album_shade(0xE61E38u);
        unsigned blue = crazypod_lm_album_shade(0x173AEAu);

        assert(red != blue);
        assert(((red >> 16) & 255u) > ((blue >> 16) & 255u));
        assert((blue & 255u) > (red & 255u));
        assert(crazypod_lm_tint_component(0u) == 8u);
        assert(crazypod_lm_tint_component(255u) == 84u);
        assert(shade_channel(255,
               crazypod_lm_tint_component(255u),
               CRAZYPOD_LM_SHADE_OPA) < 160u);
    }

    /* Native UI and wallpaper blur must crop the same iPod glass pill. */
    assert(CRAZYPOD_PRODUCT_SCREEN_WIDTH == 320);
    assert(CRAZYPOD_PRODUCT_SCREEN_HEIGHT == 240);
    assert(CRAZYPOD_PRODUCT_HOME_ROW_COUNT == 4);
    assert(CRAZYPOD_PRODUCT_HOME_VIEWPORT_Y > 28);
    assert(CRAZYPOD_PRODUCT_HOME_VIEWPORT_Y +
           CRAZYPOD_PRODUCT_HOME_VIEWPORT_HEIGHT <
           CRAZYPOD_PRODUCT_MINIPLAYER_Y);
    assert(CRAZYPOD_PRODUCT_HOME_FOCUSED_Y >=
           CRAZYPOD_PRODUCT_HOME_VIEWPORT_Y);
    assert(CRAZYPOD_PRODUCT_HOME_FOCUSED_Y +
           CRAZYPOD_PRODUCT_HOME_FOCUSED_HEIGHT <=
           CRAZYPOD_PRODUCT_HOME_VIEWPORT_Y +
           CRAZYPOD_PRODUCT_HOME_VIEWPORT_HEIGHT);
    assert(CRAZYPOD_PRODUCT_MINIPLAYER_MARGIN == 12);
    assert(CRAZYPOD_PRODUCT_MINIPLAYER_Y +
           CRAZYPOD_PRODUCT_MINIPLAYER_BOTTOM < 240);

    /* The airy concept Home still uses dark ink and readable soft glass. */
    assert(CRAZYPOD_LM_HOME_TOP != CRAZYPOD_LM_CANVAS);
    assert(CRAZYPOD_LM_HOME_SELECTED_OPA > CRAZYPOD_LM_HOME_IDLE_OPA);
    assert(CRAZYPOD_LM_HOME_SELECTED_OPA <= 240);
    assert(CRAZYPOD_LM_HOME_WALLPAPER_SCRIM >= 190);
    assert(CRAZYPOD_LM_HOME_WALLPAPER_SCRIM <= 230);
    /* Simple integer RGB channel gap; full visual contrast remains a
     * 320x240 simulator + physical LCD verification requirement. */
    assert(((CRAZYPOD_LM_HOME_GLASS >> 16) & 255u) -
           ((CRAZYPOD_LM_HOME_TEXT >> 16) & 255u) > 160u);

    /* Placeholder art must remain a quiet backdrop, not neon UI. */
    {
        static const unsigned fallback[] = CRAZYPOD_LM_FALLBACK_PALETTE;
        unsigned i;

        assert(sizeof(fallback) / sizeof(fallback[0]) == 8u);
        for(i = 0; i < 8u; ++i) {
            assert(((fallback[i] >> 16) & 255u) < 160u);
            assert(((fallback[i] >> 8) & 255u) < 160u);
            assert((fallback[i] & 255u) < 160u);
        }
    }

    /* The "glass" control material should be subtle rather than opaque. */
    assert(CRAZYPOD_LM_GLASS_FILL_OPA > 0);
    assert(CRAZYPOD_LM_GLASS_FILL_OPA < 64);
    assert(CRAZYPOD_LM_GLASS_BORDER_OPA <= 96);
    return 0;
}
