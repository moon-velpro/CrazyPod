#include "config.h"

#ifdef IPOD_6G

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "crazypod_menu_icon_assets.h"
#include "crazypod_private_icon_assets.h"

/*
 * Original CrazyPod monochrome symbols, built from small geometric strokes.
 * Optical language: Apple's regular-weight system symbols; no proprietary
 * SF Symbols paths, Apple artwork, Google Material Shapes or downloaded icons.
 *
 * Lazily rasterize one 14x14 antialiased A8 glyph per semantic base. All
 * aliases share a single persistent descriptor; the Click Wheel hot path
 * never rasterizes, scans icon files or performs drawing.
 */
#define ICON_SIZE 14
#define SUBSAMPLES 3
#define ICON_COORD_SCALE 10
#define ICON_SPAN (24 * ICON_COORD_SCALE)
#define STROKE_RADIUS 9

enum cp_glyph {
    G_MUSIC, G_PLAY, G_PODCAST, G_GEAR, G_ALBUM, G_LIBRARY,
    G_PLAYLIST, G_PERSON, G_SONG, G_SEARCH, G_QUEUE, G_SHUFFLE,
    G_REPEAT, G_SOUND, G_SUN, G_DISPLAY, G_POWER, G_SLIDERS,
    G_MENU, G_GLOBE, G_EQ, G_BATTERY, G_MOON, G_USB,
    G_STORAGE, G_CLOCK, G_GRID, G_LOCK, G_BOOK, G_NOTE,
    G_CALENDAR, G_TIMER, G_EYE, G_ARROW_UP, G_ARROW_DOWN,
    G_HEART, G_PHOTO, G_TRASH, G_CHECK, G_EDIT, G_HOME,
    G_FOLDER, G_SAVE, G_GLYPH_COUNT
};

static LV_ATTRIBUTE_MEM_ALIGN uint8_t
    icon_pixels[G_GLYPH_COUNT][ICON_SIZE * ICON_SIZE];
static lv_image_dsc_t icon_images[G_GLYPH_COUNT];
static bool icon_built[G_GLYPH_COUNT];

/* Coordinates are from a 24x24 optical grid; all arithmetic is integer.
 * Test coverage at 3x3 subpixels avoids FPU and preserves thin round caps. */
static void add_sample(uint8_t *pixels, int x, int y, int coverage)
{
    int index = y * ICON_SIZE + x;
    uint8_t alpha = (uint8_t)((coverage * 255 + 4) / 9);

    if(alpha > pixels[index])
        pixels[index] = alpha;
}

static bool sample_segment(int px, int py,
                           int x1, int y1, int x2, int y2)
{
    int64_t dx = x2 - x1;
    int64_t dy = y2 - y1;
    int64_t x = px - x1;
    int64_t y = py - y1;
    int64_t dot = x * dx + y * dy;
    int64_t len2 = dx * dx + dy * dy;

    if(dot <= 0)
        return x * x + y * y <= STROKE_RADIUS * STROKE_RADIUS;
    if(dot >= len2) {
        x = px - x2;
        y = py - y2;
        return x * x + y * y <= STROKE_RADIUS * STROKE_RADIUS;
    }
    {
        int64_t cross = x * dy - y * dx;
        return cross * cross <=
            (int64_t)STROKE_RADIUS * STROKE_RADIUS * len2;
    }
}

static void line_g(uint8_t *pixels,
                   int x1, int y1, int x2, int y2)
{
    int y;

    x1 *= ICON_COORD_SCALE;
    y1 *= ICON_COORD_SCALE;
    x2 *= ICON_COORD_SCALE;
    y2 *= ICON_COORD_SCALE;
    for(y = 0; y < ICON_SIZE; ++y) {
        int x;

        for(x = 0; x < ICON_SIZE; ++x) {
            int hit = 0;
            int sy;

            for(sy = 0; sy < SUBSAMPLES; ++sy) {
                int sx;
                int py = ((2 * (y * SUBSAMPLES + sy) + 1) *
                    ICON_SPAN) / (2 * ICON_SIZE * SUBSAMPLES);

                for(sx = 0; sx < SUBSAMPLES; ++sx) {
                    int px = ((2 * (x * SUBSAMPLES + sx) + 1) *
                        ICON_SPAN) / (2 * ICON_SIZE * SUBSAMPLES);

                    if(sample_segment(px, py, x1, y1, x2, y2))
                        ++hit;
                }
            }
            add_sample(pixels, x, y, hit);
        }
    }
}

static void circle_g(uint8_t *pixels,
                     int cx, int cy, int radius, bool solid,
                     int upper_limit)
{
    int y;
    int outer;
    int inner;

    cx *= ICON_COORD_SCALE;
    cy *= ICON_COORD_SCALE;
    outer = (radius * ICON_COORD_SCALE) +
        (solid ? 0 : STROKE_RADIUS);
    inner = solid ? 0 : radius * ICON_COORD_SCALE - STROKE_RADIUS;
    if(inner < 0)
        inner = 0;
    for(y = 0; y < ICON_SIZE; ++y) {
        int x;

        for(x = 0; x < ICON_SIZE; ++x) {
            int hit = 0;
            int sy;

            for(sy = 0; sy < SUBSAMPLES; ++sy) {
                int sx;
                int py = ((2 * (y * SUBSAMPLES + sy) + 1) *
                    ICON_SPAN) / (2 * ICON_SIZE * SUBSAMPLES);

                if(upper_limit >= 0 &&
                   py > upper_limit * ICON_COORD_SCALE)
                    continue;
                for(sx = 0; sx < SUBSAMPLES; ++sx) {
                    int px = ((2 * (x * SUBSAMPLES + sx) + 1) *
                        ICON_SPAN) / (2 * ICON_SIZE * SUBSAMPLES);
                    int64_t dx = px - cx;
                    int64_t dy = py - cy;
                    int64_t d2 = dx * dx + dy * dy;

                    if(d2 <= (int64_t)outer * outer &&
                       d2 >= (int64_t)inner * inner)
                        ++hit;
                }
            }
            add_sample(pixels, x, y, hit);
        }
    }
}

static void triangle_g(uint8_t *pixels,
                       int ax, int ay, int bx, int by,
                       int cx, int cy)
{
    int y;

    ax *= ICON_COORD_SCALE;
    ay *= ICON_COORD_SCALE;
    bx *= ICON_COORD_SCALE;
    by *= ICON_COORD_SCALE;
    cx *= ICON_COORD_SCALE;
    cy *= ICON_COORD_SCALE;
    for(y = 0; y < ICON_SIZE; ++y) {
        int x;

        for(x = 0; x < ICON_SIZE; ++x) {
            int hit = 0;
            int sy;

            for(sy = 0; sy < SUBSAMPLES; ++sy) {
                int sx;
                int py = ((2 * (y * SUBSAMPLES + sy) + 1) *
                    ICON_SPAN) / (2 * ICON_SIZE * SUBSAMPLES);

                for(sx = 0; sx < SUBSAMPLES; ++sx) {
                    int px = ((2 * (x * SUBSAMPLES + sx) + 1) *
                        ICON_SPAN) / (2 * ICON_SIZE * SUBSAMPLES);
                    int64_t d1 = (int64_t)(bx-ax)*(py-ay) -
                        (int64_t)(by-ay)*(px-ax);
                    int64_t d2 = (int64_t)(cx-bx)*(py-by) -
                        (int64_t)(cy-by)*(px-bx);
                    int64_t d3 = (int64_t)(ax-cx)*(py-cy) -
                        (int64_t)(ay-cy)*(px-cx);
                    if((d1>=0 && d2>=0 && d3>=0) ||
                       (d1<=0 && d2<=0 && d3<=0))
                        ++hit;
                }
            }
            add_sample(pixels, x, y, hit);
        }
    }
}

static void rect_g(uint8_t *p, int x1, int y1, int x2, int y2)
{
    line_g(p,x1,y1,x2,y1);
    line_g(p,x2,y1,x2,y2);
    line_g(p,x2,y2,x1,y2);
    line_g(p,x1,y2,x1,y1);
}

static void draw_glyph(enum cp_glyph glyph, uint8_t *p)
{
    int i;

    switch(glyph) {
    case G_MUSIC:
        line_g(p,15,5,15,18);
        line_g(p,15,5,20,4);
        circle_g(p,12,18,3,true,-1);
        break;
    case G_PLAY:
        circle_g(p,12,12,9,false,-1);
        triangle_g(p,10,7,17,12,10,17);
        break;
    case G_PODCAST:
        circle_g(p,12,12,9,false,13);
        circle_g(p,12,12,6,false,12);
        circle_g(p,12,12,2,true,-1);
        line_g(p,12,15,12,20);
        break;
    case G_GEAR:
        circle_g(p,12,12,7,false,-1);
        circle_g(p,12,12,2,false,-1);
        for(i=0;i<8;++i) {
            static const int8_t radial[8][4] = {
                {12,2,12,5}, {19,5,17,7},
                {22,12,19,12}, {19,19,17,17},
                {12,22,12,19}, {5,19,7,17},
                {2,12,5,12}, {5,5,7,7}
            };
            line_g(p, radial[i][0],radial[i][1],
                      radial[i][2],radial[i][3]);
        }
        break;
    case G_ALBUM:
        rect_g(p,4,5,18,21);
        line_g(p,7,5,7,3);
        line_g(p,7,3,20,3);
        circle_g(p,11,13,3,false,-1);
        break;
    case G_LIBRARY:
        rect_g(p,3,3,21,21);
        line_g(p,9,7,9,17);
        line_g(p,9,7,17,6);
        circle_g(p,7,17,2,true,-1);
        break;
    case G_PLAYLIST:
        line_g(p,4,5,19,5);
        line_g(p,4,10,19,10);
        line_g(p,4,16,12,16);
        circle_g(p,15,19,2,true,-1);
        line_g(p,17,19,17,14);
        break;
    case G_PERSON:
        circle_g(p,12,8,4,false,-1);
        line_g(p,4,21,6,17);
        line_g(p,6,17,9,15);
        line_g(p,9,15,15,15);
        line_g(p,15,15,18,17);
        line_g(p,18,17,20,21);
        break;
    case G_SONG:
        rect_g(p,5,3,19,21);
        line_g(p,13,10,13,17);
        line_g(p,13,10,17,9);
        circle_g(p,10,17,2,true,-1);
        break;
    case G_SEARCH:
        circle_g(p,10,10,6,false,-1);
        line_g(p,15,15,21,21);
        break;
    case G_QUEUE:
        line_g(p,3,5,20,5);
        line_g(p,3,10,20,10);
        line_g(p,3,16,12,16);
        triangle_g(p,15,14,21,18,15,22);
        break;
    case G_SHUFFLE:
        line_g(p,3,5,7,5);
        line_g(p,7,5,17,19);
        line_g(p,17,19,21,19);
        line_g(p,3,19,7,19);
        line_g(p,7,19,17,5);
        line_g(p,17,5,21,5);
        line_g(p,18,2,21,5);
        line_g(p,21,5,18,8);
        line_g(p,18,16,21,19);
        line_g(p,21,19,18,22);
        break;
    case G_REPEAT:
        line_g(p,4,10,4,6);
        line_g(p,4,6,20,6);
        line_g(p,20,6,20,11);
        line_g(p,20,14,20,18);
        line_g(p,20,18,4,18);
        line_g(p,4,18,4,13);
        line_g(p,17,8,20,11);
        line_g(p,20,11,23,8);
        line_g(p,7,16,4,13);
        line_g(p,4,13,1,16);
        break;
    case G_SOUND:
        line_g(p,3,10,7,10);
        line_g(p,7,10,12,6);
        line_g(p,12,6,12,18);
        line_g(p,12,18,7,14);
        line_g(p,7,14,3,14);
        line_g(p,3,14,3,10);
        line_g(p,15,8,17,11);
        line_g(p,17,11,17,13);
        line_g(p,17,13,15,16);
        line_g(p,18,5,21,9);
        line_g(p,21,9,21,15);
        line_g(p,21,15,18,19);
        break;
    case G_SUN:
        circle_g(p,12,12,4,false,-1);
        {
            static const int8_t rays[8][4] = {
                {12,2,12,5},{19,5,17,7},{22,12,19,12},
                {19,19,17,17},{12,22,12,19},{5,19,7,17},
                {2,12,5,12},{5,5,7,7}
            };
            for(i=0;i<8;++i)
                line_g(p,rays[i][0],rays[i][1],
                         rays[i][2],rays[i][3]);
        }
        break;
    case G_DISPLAY:
        rect_g(p,3,4,21,17);
        line_g(p,12,17,12,21);
        line_g(p,7,21,17,21);
        break;
    case G_POWER:
        circle_g(p,12,13,8,false,-1);
        line_g(p,12,2,12,12);
        break;
    case G_SLIDERS:
        line_g(p,4,6,20,6);
        line_g(p,4,12,20,12);
        line_g(p,4,18,20,18);
        circle_g(p,9,6,2,true,-1);
        circle_g(p,16,12,2,true,-1);
        circle_g(p,10,18,2,true,-1);
        break;
    case G_MENU:
        line_g(p,4,6,20,6);
        line_g(p,4,12,20,12);
        line_g(p,4,18,20,18);
        break;
    case G_GLOBE:
        circle_g(p,12,12,9,false,-1);
        line_g(p,3,12,21,12);
        line_g(p,12,3,12,21);
        break;
    case G_EQ:
        for(i=0;i<5;++i) {
            static const int8_t heights[5] = {4,8,6,9,5};
            int x=4+i*4;
            line_g(p,x,12-heights[i],x,12+heights[i]);
        }
        break;
    case G_BATTERY:
        rect_g(p,3,6,19,18);
        line_g(p,21,10,21,14);
        line_g(p,13,8,9,13);
        line_g(p,9,13,13,13);
        line_g(p,13,13,11,17);
        break;
    case G_MOON:
        circle_g(p,11,12,9,false,-1);
        circle_g(p,16,7,7,false,13);
        break;
    case G_USB:
        line_g(p,12,21,12,4);
        line_g(p,12,12,6,12);
        line_g(p,12,16,18,16);
        triangle_g(p,9,6,15,6,12,2);
        circle_g(p,6,12,2,true,-1);
        rect_g(p,17,14,20,18);
        break;
    case G_STORAGE:
        rect_g(p,3,4,21,20);
        line_g(p,5,15,19,15);
        circle_g(p,17,18,1,true,-1);
        break;
    case G_CLOCK:
        circle_g(p,12,12,9,false,-1);
        line_g(p,12,6,12,12);
        line_g(p,12,12,16,15);
        break;
    case G_GRID:
        rect_g(p,3,3,10,10);
        rect_g(p,14,3,21,10);
        rect_g(p,3,14,10,21);
        rect_g(p,14,14,21,21);
        break;
    case G_LOCK:
        rect_g(p,4,10,20,21);
        circle_g(p,12,10,6,false,11);
        line_g(p,6,10,6,7);
        line_g(p,18,10,18,7);
        break;
    case G_BOOK:
        line_g(p,12,5,5,3);
        line_g(p,5,3,3,4);
        line_g(p,3,4,3,19);
        line_g(p,3,19,12,21);
        line_g(p,12,21,21,19);
        line_g(p,21,19,21,4);
        line_g(p,21,4,19,3);
        line_g(p,19,3,12,5);
        line_g(p,12,5,12,21);
        break;
    case G_NOTE:
        rect_g(p,4,3,20,21);
        line_g(p,7,8,17,8);
        line_g(p,7,12,17,12);
        line_g(p,7,16,14,16);
        break;
    case G_CALENDAR:
        rect_g(p,3,5,21,21);
        line_g(p,3,10,21,10);
        line_g(p,8,3,8,7);
        line_g(p,16,3,16,7);
        circle_g(p,9,15,1,true,-1);
        circle_g(p,15,15,1,true,-1);
        break;
    case G_TIMER:
        circle_g(p,12,13,8,false,-1);
        line_g(p,12,13,16,9);
        line_g(p,12,2,12,5);
        line_g(p,8,2,16,2);
        break;
    case G_EYE:
        line_g(p,2,12,7,7);
        line_g(p,7,7,12,5);
        line_g(p,12,5,17,7);
        line_g(p,17,7,22,12);
        line_g(p,22,12,17,17);
        line_g(p,17,17,12,19);
        line_g(p,12,19,7,17);
        line_g(p,7,17,2,12);
        circle_g(p,12,12,3,false,-1);
        break;
    case G_ARROW_UP:
        line_g(p,12,21,12,3);
        line_g(p,5,10,12,3);
        line_g(p,12,3,19,10);
        break;
    case G_ARROW_DOWN:
        line_g(p,12,3,12,21);
        line_g(p,5,14,12,21);
        line_g(p,12,21,19,14);
        break;
    case G_HEART:
        line_g(p,12,20,4,12);
        line_g(p,4,12,4,7);
        line_g(p,4,7,7,5);
        line_g(p,7,5,10,6);
        line_g(p,10,6,12,8);
        line_g(p,12,8,14,6);
        line_g(p,14,6,17,5);
        line_g(p,17,5,20,7);
        line_g(p,20,7,20,12);
        line_g(p,20,12,12,20);
        break;
    case G_PHOTO:
        rect_g(p,3,4,21,20);
        circle_g(p,8,9,2,true,-1);
        line_g(p,4,18,10,12);
        line_g(p,10,12,14,16);
        line_g(p,14,16,17,13);
        line_g(p,17,13,20,17);
        break;
    case G_TRASH:
        rect_g(p,6,7,18,21);
        line_g(p,4,6,20,6);
        line_g(p,10,4,14,4);
        line_g(p,10,10,10,18);
        line_g(p,14,10,14,18);
        break;
    case G_CHECK:
        line_g(p,4,12,10,18);
        line_g(p,10,18,20,6);
        break;
    case G_EDIT:
        line_g(p,5,20,6,14);
        line_g(p,6,14,18,3);
        line_g(p,18,3,21,6);
        line_g(p,21,6,9,19);
        line_g(p,9,19,5,20);
        break;
    case G_HOME:
        line_g(p,3,11,12,3);
        line_g(p,12,3,21,11);
        line_g(p,5,10,5,21);
        line_g(p,19,10,19,21);
        line_g(p,5,21,19,21);
        rect_g(p,10,14,14,21);
        break;
    case G_FOLDER:
        line_g(p,3,7,9,7);
        line_g(p,9,7,12,10);
        line_g(p,12,10,21,10);
        line_g(p,21,10,21,20);
        line_g(p,21,20,3,20);
        line_g(p,3,20,3,7);
        break;
    case G_SAVE:
        rect_g(p,4,3,20,21);
        rect_g(p,8,3,16,9);
        rect_g(p,8,14,16,21);
        break;
    case G_GLYPH_COUNT:
        break;
    }
}

static enum cp_glyph menu_glyph(enum crazypod_menu_icon icon)
{
    switch(icon) {
    case CRAZYPOD_MENU_ICON_MUSIC:
        return G_MUSIC;
    case CRAZYPOD_MENU_ICON_NOW_PLAYING:
    case CRAZYPOD_MENU_ICON_PLAYBACK:
    case CRAZYPOD_MENU_ICON_START:
        return G_PLAY;
    case CRAZYPOD_MENU_ICON_PODCAST:
        return G_PODCAST;
    case CRAZYPOD_MENU_ICON_SETTINGS:
        return G_GEAR;
    case CRAZYPOD_MENU_ICON_ALBUM:
    case CRAZYPOD_MENU_ICON_ALBUM_FLOW:
        return G_ALBUM;
    case CRAZYPOD_MENU_ICON_MUSIC_LIBRARY:
        return G_LIBRARY;
    case CRAZYPOD_MENU_ICON_PLAYLIST:
        return G_PLAYLIST;
    case CRAZYPOD_MENU_ICON_ARTIST:
    case CRAZYPOD_MENU_ICON_CONTACT:
        return G_PERSON;
    case CRAZYPOD_MENU_ICON_SONG:
        return G_SONG;
    case CRAZYPOD_MENU_ICON_SEARCH:
        return G_SEARCH;
    case CRAZYPOD_MENU_ICON_QUEUE:
        return G_QUEUE;
    case CRAZYPOD_MENU_ICON_SHUFFLE:
        return G_SHUFFLE;
    case CRAZYPOD_MENU_ICON_REPEAT:
    case CRAZYPOD_MENU_ICON_REPEAT_CLICKS:
        return G_REPEAT;
    case CRAZYPOD_MENU_ICON_SOUND:
    case CRAZYPOD_MENU_ICON_SPEAKER:
        return G_SOUND;
    case CRAZYPOD_MENU_ICON_BRIGHTNESS:
    case CRAZYPOD_MENU_ICON_BACKLIGHT:
    case CRAZYPOD_MENU_ICON_GLOW:
        return G_SUN;
    case CRAZYPOD_MENU_ICON_DISPLAY:
    case CRAZYPOD_MENU_ICON_DISPLAY_SLEEP:
        return G_DISPLAY;
    case CRAZYPOD_MENU_ICON_POWER:
    case CRAZYPOD_MENU_ICON_POWER_TIMER:
        return G_POWER;
    case CRAZYPOD_MENU_ICON_CONTROLS:
    case CRAZYPOD_MENU_ICON_DETAILS:
    case CRAZYPOD_MENU_ICON_BALANCE:
        return G_SLIDERS;
    case CRAZYPOD_MENU_ICON_MENU:
        return G_MENU;
    case CRAZYPOD_MENU_ICON_LANGUAGE:
        return G_GLOBE;
    case CRAZYPOD_MENU_ICON_EQUALIZER:
    case CRAZYPOD_MENU_ICON_BASS:
    case CRAZYPOD_MENU_ICON_TREBLE:
    case CRAZYPOD_MENU_ICON_WAVE:
    case CRAZYPOD_MENU_ICON_STATS:
    case CRAZYPOD_MENU_ICON_SUMMARY:
        return G_EQ;
    case CRAZYPOD_MENU_ICON_CHARGING:
        return G_BATTERY;
    case CRAZYPOD_MENU_ICON_SLEEP_TIMER:
    case CRAZYPOD_MENU_ICON_COOLDOWN:
        return G_MOON;
    case CRAZYPOD_MENU_ICON_USB:
        return G_USB;
    case CRAZYPOD_MENU_ICON_STORAGE:
        return G_STORAGE;
    case CRAZYPOD_MENU_ICON_CLOCK:
    case CRAZYPOD_MENU_ICON_TIME:
    case CRAZYPOD_MENU_ICON_TIMER_BOOT:
    case CRAZYPOD_MENU_ICON_RESET_TIMER:
    case CRAZYPOD_MENU_ICON_RECENTS:
    case CRAZYPOD_MENU_ICON_HISTORY:
        return G_CLOCK;
    case CRAZYPOD_MENU_ICON_APPS:
    case CRAZYPOD_MENU_ICON_MORE:
    case CRAZYPOD_MENU_ICON_ICONS:
    case CRAZYPOD_MENU_ICON_LAYOUT:
        return G_GRID;
    case CRAZYPOD_MENU_ICON_LOCK:
        return G_LOCK;
    case CRAZYPOD_MENU_ICON_BOOK:
    case CRAZYPOD_MENU_ICON_READING:
    case CRAZYPOD_MENU_ICON_CHAPTERS:
        return G_BOOK;
    case CRAZYPOD_MENU_ICON_NOTE:
    case CRAZYPOD_MENU_ICON_ADD_NOTE:
    case CRAZYPOD_MENU_ICON_DRAFT:
    case CRAZYPOD_MENU_ICON_TITLE:
    case CRAZYPOD_MENU_ICON_TEXT_SIZE:
        return G_NOTE;
    case CRAZYPOD_MENU_ICON_CALENDAR:
    case CRAZYPOD_MENU_ICON_DATE:
    case CRAZYPOD_MENU_ICON_TODAY:
    case CRAZYPOD_MENU_ICON_UPCOMING:
    case CRAZYPOD_MENU_ICON_MONTH:
    case CRAZYPOD_MENU_ICON_ADD_EVENT:
    case CRAZYPOD_MENU_ICON_EVENT:
        return G_CALENDAR;
    case CRAZYPOD_MENU_ICON_STOPWATCH:
        return G_TIMER;
    case CRAZYPOD_MENU_ICON_VISIBILITY:
    case CRAZYPOD_MENU_ICON_MOTION_OFF:
        return G_EYE;
    case CRAZYPOD_MENU_ICON_MOVE_UP:
    case CRAZYPOD_MENU_ICON_TOP:
        return G_ARROW_UP;
    case CRAZYPOD_MENU_ICON_MOVE_DOWN:
    case CRAZYPOD_MENU_ICON_BOTTOM:
        return G_ARROW_DOWN;
    case CRAZYPOD_MENU_ICON_FAVORITE:
        return G_HEART;
    case CRAZYPOD_MENU_ICON_PHOTO:
    case CRAZYPOD_MENU_ICON_VIDEO:
    case CRAZYPOD_MENU_ICON_WALLPAPER:
    case CRAZYPOD_MENU_ICON_BACKGROUNDS:
        return G_PHOTO;
    case CRAZYPOD_MENU_ICON_TRASH:
    case CRAZYPOD_MENU_ICON_ERASE:
    case CRAZYPOD_MENU_ICON_RESTORE:
        return G_TRASH;
    case CRAZYPOD_MENU_ICON_CHECK:
    case CRAZYPOD_MENU_ICON_APPLY:
        return G_CHECK;
    case CRAZYPOD_MENU_ICON_EDIT:
    case CRAZYPOD_MENU_ICON_HIGHLIGHT:
        return G_EDIT;
    case CRAZYPOD_MENU_ICON_HOME:
        return G_HOME;
    case CRAZYPOD_MENU_ICON_SAVE:
    case CRAZYPOD_MENU_ICON_EXPORT:
    case CRAZYPOD_MENU_ICON_IMPORT:
        return G_SAVE;
    case CRAZYPOD_MENU_ICON_PIN:
    case CRAZYPOD_MENU_ICON_PRESETS:
    case CRAZYPOD_MENU_ICON_BOOKMARK:
    case CRAZYPOD_MENU_ICON_DUPLICATE:
        return G_FOLDER;
    case CRAZYPOD_MENU_ICON_CUSTOMIZE:
    case CRAZYPOD_MENU_ICON_PAGE_THEME:
    case CRAZYPOD_MENU_ICON_THEMES:
    case CRAZYPOD_MENU_ICON_PRIMARY_COLOR:
    case CRAZYPOD_MENU_ICON_SECONDARY_COLOR:
        return G_SUN;
    case CRAZYPOD_MENU_ICON_WORKOUT:
    case CRAZYPOD_MENU_ICON_RUN:
    case CRAZYPOD_MENU_ICON_WALK:
    case CRAZYPOD_MENU_ICON_CYCLING:
    case CRAZYPOD_MENU_ICON_HIKING:
    case CRAZYPOD_MENU_ICON_STAIRS:
    case CRAZYPOD_MENU_ICON_ROWING:
    case CRAZYPOD_MENU_ICON_STRENGTH:
    case CRAZYPOD_MENU_ICON_YOGA:
    case CRAZYPOD_MENU_ICON_TENNIS:
    case CRAZYPOD_MENU_ICON_BASKETBALL:
    case CRAZYPOD_MENU_ICON_SOCCER:
        return G_PERSON;
    case CRAZYPOD_MENU_ICON_KEYCLICK:
    case CRAZYPOD_MENU_ICON_BEEP:
    case CRAZYPOD_MENU_ICON_ICON_SIZE:
        return G_SLIDERS;
    case CRAZYPOD_MENU_ICON_NONE:
    case CRAZYPOD_MENU_ICON_COUNT:
        break;
    }
    return G_MENU;
}

const lv_image_dsc_t *crazypod_menu_icon_asset(
    enum crazypod_menu_icon icon)
{
    enum cp_glyph glyph;
    lv_image_dsc_t *image;

    const lv_image_dsc_t *private_icon;

    if(icon <= CRAZYPOD_MENU_ICON_NONE ||
       icon >= CRAZYPOD_MENU_ICON_COUNT)
        return NULL;
    private_icon = crazypod_private_icon_menu(icon);
    if(private_icon != NULL)
        return private_icon;
    glyph = menu_glyph(icon);
    image = &icon_images[glyph];
    if(!icon_built[glyph]) {
        memset(image, 0, sizeof(*image));
        memset(icon_pixels[glyph], 0, sizeof(icon_pixels[glyph]));
        draw_glyph(glyph, icon_pixels[glyph]);
        image->header.magic = LV_IMAGE_HEADER_MAGIC;
        image->header.cf = LV_COLOR_FORMAT_A8;
        image->header.w = ICON_SIZE;
        image->header.h = ICON_SIZE;
        image->header.stride = ICON_SIZE;
        image->data_size = sizeof(icon_pixels[glyph]);
        image->data = icon_pixels[glyph];
        icon_built[glyph] = true;
    }
    return image;
}

#endif
