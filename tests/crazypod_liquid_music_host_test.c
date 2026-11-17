#include <assert.h>

#include "../apps/crazypod/ui/presentation/crazypod_liquid_music.h"

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

    /* The "glass" control material should be subtle rather than opaque. */
    assert(CRAZYPOD_LM_GLASS_FILL_OPA > 0);
    assert(CRAZYPOD_LM_GLASS_FILL_OPA < 64);
    assert(CRAZYPOD_LM_GLASS_BORDER_OPA <= 96);
    return 0;
}
