#ifndef CRAZYPOD_LIQUID_MUSIC_H
#define CRAZYPOD_LIQUID_MUSIC_H

/*
 * CrazyPod 2026 visual tokens: content-first and Click-Wheel-native.
 * This header is intentionally LVGL-independent for host validation.
 *
 * "Glass" here means a static, contrast-led control material. The iPod
 * does not implement per-frame refraction or backdrop blur.
 */
#define CRAZYPOD_LM_CANVAS                 0x111319u
#define CRAZYPOD_LM_TEXT                   0xFFFFFFu
#define CRAZYPOD_LM_TEXT_MUTED             0xCBD3DDu
#define CRAZYPOD_LM_GLASS                  0xC8D4E5u
#define CRAZYPOD_LM_GLASS_FILL_OPA         38
#define CRAZYPOD_LM_GLASS_BORDER_OPA       56
#define CRAZYPOD_LM_FOCUS_FILL_OPA         76
#define CRAZYPOD_LM_FOCUS_BORDER_OPA       93

/* Shared 320x240 Apple-like presentation. Glass belongs to controls and
 * the mini player; long lists are clean paper with quiet separators. */
#define CRAZYPOD_LM_NATIVE_BG              0xF8F9FBu
#define CRAZYPOD_LM_NATIVE_INK             0x18212Du
#define CRAZYPOD_LM_NATIVE_SECONDARY       0x6C7481u
#define CRAZYPOD_LM_NATIVE_SELECTED        0xDCE8FCu
#define CRAZYPOD_LM_NATIVE_HAIRLINE        0xD9DEE7u

/* iPod 2026 Home: calm near-white gradient, dark labels, a single
 * highlighted row instead of four independent outlined glass pills. */
#define CRAZYPOD_LM_HOME_TOP               0xF9FAFDu
#define CRAZYPOD_LM_HOME_BOTTOM            0xE9EEF7u
#define CRAZYPOD_LM_HOME_TEXT              0x18212Du
#define CRAZYPOD_LM_HOME_MUTED             0x3E4754u
#define CRAZYPOD_LM_HOME_GLASS             0xFFFFFFu
#define CRAZYPOD_LM_HOME_SELECTION_FILL    0xDDE8FCu
#define CRAZYPOD_LM_HOME_SELECTED_OPA      205
#define CRAZYPOD_LM_HOME_IDLE_OPA          0
#define CRAZYPOD_LM_HOME_RIM_OPA           100
#define CRAZYPOD_LM_HOME_WALLPAPER_SCRIM   215

/* One artwork dimension is shared by the screen and its RGB565 cache.
 * Keep the presentation's double-bank allocation in the memory budget. */
#define CRAZYPOD_LM_ARTWORK_SIZE           128
#define CRAZYPOD_LM_ARTWORK_X              16
#define CRAZYPOD_LM_ARTWORK_Y              43
#define CRAZYPOD_LM_INFO_X                 160
#define CRAZYPOD_LM_INFO_WIDTH             144

/* Cached artwork-derived backgrounds are shaded once by LVGL on display.
 * Keep the presentation's contrast model and fallback model synchronized. */
#define CRAZYPOD_LM_SHADE_OPA              150

/* Muted, real-device-friendly fallback colors only when cover data
 * is absent. Cached artwork always takes priority over these tones. */
#define CRAZYPOD_LM_FALLBACK_PALETTE { \
    0x82796Fu, 0x60788Cu, 0x826F7Bu, 0x738478u, \
    0x756E66u, 0x728396u, 0x8B7277u, 0x667C83u  \
}

/* Compact waveform/scrubber sits above physical-button playback glyphs.
 * No new audio visualization thread or touch controls are introduced. */
#define CRAZYPOD_LM_WAVE_X                 16
#define CRAZYPOD_LM_WAVE_Y                 181
#define CRAZYPOD_LM_WAVE_WIDTH             288
#define CRAZYPOD_LM_WAVE_HEIGHT             8

#endif
