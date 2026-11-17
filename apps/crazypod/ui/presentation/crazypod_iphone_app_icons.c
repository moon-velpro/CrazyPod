#include "config.h"

#ifdef IPOD_6G

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "crazypod_iphone_app_icons.h"

/*
 * Four 21x21 source-derived icons supplied by the project owner as PNGs
 * in the design discussion, resized with Lanczos and packed as little-endian
 * RGB565 color plane followed by A8 alpha plane (LVGL native format).
 * The icon silhouettes, glints and soft rounded corners are preserved.
 *
 * Source order: Now Playing=upload 4, Music=upload 1,
 * Podcasts=upload 2, Settings=upload 3.
 * Asset redistribution rights for public firmware releases must be checked.
 *
 * Base64 is decoded once per requested icon, not per repaint/detent.
 * The 4-frame resident budget is 4 * 21 * 21 * 3 = 5,292 bytes.
 */
#define APP_SIDE CRAZYPOD_IPHONE_APP_ICON_SIZE
#define APP_PIXELS (APP_SIDE * APP_SIDE)
#define APP_BYTES (APP_PIXELS * 3)
#define APP_COUNT CRAZYPOD_HOME_DESTINATION_COUNT

static const char *const app_base64[APP_COUNT] = {
    /* Now Playing <- uploaded icon 4 (white waveform). */
    "//8AAP////////////////////////////////////////////8AAP//////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////////////////////////////////////////"
    "3////////////////////1T+Ov//////////////////////////////3///////3////9/////f/0j08/3//zv/3v/f/9//3//f/9//3//f"
    "/9//3//f/9/////f/9//3//e/wv09fWe/8nzuf7/////3//f/9//3//f/9//3//f/9/////f/9//3/++/6vz1fV9/ynzmfY8/7D0nf//////"
    "3//f/9//3//f/9//3//f/9//3/++/0zzlfV9/8ryuvZ59qnyPPc8/3n23///////vv/f/9//3/++97733/e+9+3yVvVd92vymvZ59mvyffcV"
    "9e3y3/fb9hz33/e+9773vve+9773vvee943yNvVd9wzyevY69gzyXff19K7y3/cw8xL0//ee9773vve+9773vvee94/yN/U99w3qWvY69i7q"
    "Xff29K/qvvfb9tz2vve+9773vvee9573vvee9/HqV+0993Dqe/Y69i/qHPcc9zr2nvfe9773nvee9773vvee9573nvee9zPrWO0995LiW+78"
    "9nbsPfe+9773nvee9573nvee99//nvee9573nvee93Xjee1d9zXjW+7e9773nvee9373nvee9573nvee99//fe+e9373nvee93fjWuW+99zu"
    "Xe+e9333nvee9573nvee9573fe++9573fe99733vfe+d77vlnO6d757vfe99733vfe99733vfe99733vnvff/33vfe++933vfe99753vfe99"
    "733vfe99733vfe99733vfe997573v/e+933vXe///13vXe9d713vXe99733vfe99733vfe99713vXe+e9773fe///13v//9d7///nvd9713v"
    "Xe9d713vXe9d713vXe9d733vvve+9573//9d7/////8AAJ73nvd+713vXe99733vfe99733vfu+e9773nvd97573nvcAAP//AQBz0vT9////"
    "/////////fTScwABBKb//////////////////////6YEdP/7/f/////////////////9+/9z2v39/////////////////////f3a9///////"
    "///////////////////3/v/////////////////////////+////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////////////////////////////////////////"
    "/////////////////////////////////////////////////////////////////////////////////////v//////////////////////"
    "///+9//////////////////////////32v39/////////////////////f3adP/7/f/////////////////9+/90BKb/////////////////"
    "/////6YEAQBy0vP+/////////////fPScgAB",
    /* Music <- uploaded icon 1 (black music note on white). */
    "//8AAP////////////////////////////////////////////8AAP/////////////////////////////////////////////////////f"
    "//e9////////////////////////3/////////////veXe///////////////////////////////////////////zznFKXLWmEI05z//9//"
    "/////9///////////////////////13vVa3rWuMYAAAAAAAA05z//9///////9//3//f/9//3//f/9////9VreMYAAAAAAAAQQggAAAA05z/"
    "/9//3//f/9//3//f/9//3//f/773///PewAAQQggAAAAAACCEGEIspT//7733//f/9//3//f/9//3//f/773//8QhAAAAABBCKYxz3u61gxj"
    "UYz//7733//f/9//3//f/9//3//f/773///ve8MYz3v3vX3v/////01rMIT//7733//f/9//3/++9773vve+9573//+Oc897///f/773nvff"
    "/01rMIT//773vve+9773vve+9773vve+9573//+Oc+97//+e9573nvf//45zMIT//573vve+9773vve+9773vve+9573//+Oc897//+e9///"
    "nvea1opScYz//573vve+9773vvee9573nvee933v//+Oc897///f/1nOCELDGGEI05z//573nvee9773nvee9573nvee99////+Oc+97//9d"
    "7yhCYQgEIeMYda3f/573nvee9773nvee9573nvee99Ocy1okIXGM///73qYx4xgEIctaHOee9573nvee9573fe+e933vvvfve2EIohDDGNOc"
    "3/+e9xSlLGNxjNvenvd97573nvee9573fe997573HOfHOSQhRSmmMTjGnvd97773nve+9573fe99733vfe997573fe+e933vfe+Nc6YxaUo0"
    "pX3vfe99713vfe9d733vfe99733vfe+e9573Xe///13vfe885zjG2959713vfe99733vfe99733vfe99713vXe///573971d7///nvd+7573"
    "fe9d713vXe9d713vXe9d713vXe997573//+e9/////8AAH3vnvd9713vXe9d733vfe99733vfe99733vfe+e97733/8AAP//AQBz0vT9////"
    "/////////fTScwABBKb//////////////////////6YEdP/7/f/////////////////9+/9z2v39/////////////////////f3a9///////"
    "///////////////////3/v/////////////////////////+////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////////////////////////////////////////"
    "/////////////////////////////////////////////////////////////////////////////////////v//////////////////////"
    "///+9//////////////////////////32v39/////////////////////f3adP/7/f/////////////////9+/90BKb/////////////////"
    "/////6YEAQBy0vP+/////////////fPScgAB",
    /* Podcasts <- uploaded icon 2 (violet podcast artwork). */
    "AAAgADzEO7PZqrmimaKZopmieaJZmlmaWZo4mhiaGJr4kfmZGJoAAAAA//+as1qiuJG4kbiRuJGXiZeJuJHYkbiRl4mXidiR2JHYkdiR+qH4"
    "kfe5O7xaopeJ2JHYkbiR2JGaqjy7fcN9w13DHLt6oreRuJHYkdiRuJH6mRiSGqu4kdiR2JG3iRiaXMOewz27Hbv8sh27Pbt+wxy7+JG3idiR"
    "15HYkfiR2aK3ideRt4kYmp3LPbvcsvyyXLudw1y73LLcsj27Pbv4kbeJ15HXkdeRmJq3ideJ14l8wz273LJ8u57c3+zf7N/sftxcu9uyPbsc"
    "s7eJ14nXideJmJq3iZaBuqp9w9uqfLv/7J/kP9wf3D/cn+Sf7Dy727Jdu1malonXideJeJqWibaJPLscs9uqntx/5B/c3+S/7b/kH9yf5D7c"
    "26ocs9uqlom3ideJeJqWgfeRXLvbqly7v+Qf3J/cf//f/1//f9w/3H/kG7PbshyztomWgfeJV5J1gReSXLu7qny7n+T/01/l3/8/97//H+X/"
    "03/kG7O7qhyz14mWgfeJV5J1gfeRPLu6qjuzf+T/01/Uv/Zf/3/2H9Qf1F/c+6raqvuytomWgRaKNpKVgbaBG7PbqrqiPtQ/1L/LX9Sf5T7U"
    "v8s/3P7LmaLbqtqqlYGVgRaKFoqVgZV5uaIbs5miG7M/3J/c//Yf99/2f9wf1PqqmaL7qliadXmVgRaK9omVgZWB1okbs9qquaK7ql7dP/+f"
    "7j//Ht2aopmi2qr6qpWBlYGVgRaK9YGVeZWBdHk3khuz2qJZmjvEnu4d3n7m2rM4mtqq+6r3iXR5tYF1eTaK1YGUeZR5lYF0eTeS+qqaovu7"
    "Pua81R3emrOaovqq9olUeZWBlHl0eTaKtYGUeZR5lHmUeXR51YFYmnqzHuad1d3dOqtYmrWBVHGUeZR5lHl0eVaKtHmVgZR5lHmUeZR5dHlU"
    "cdWBG807zdrElHlTcXR5lHmUeZR5lHl1eXaS1IG2gXR5lHmUeZR5lHmUeVNx95qavNaSM3GUeZR5lHmUeZR5VHH2iTej93m0ebaBlHmUeZR5"
    "lHl0eXR5U3F0cTNxdHl0eXR5dHl0eXR59onWkv//AAAAANWBtYG0eZR5lHm0ebR51YH1gfWB9YEVghWCFYo2ipaSWKMgAAAAAQBz0vT9////"
    "/////////fTScwABBKb//////////////////////6YEdP/7/f/////////////////9+/9z2v39/////////////////////f3a9///////"
    "///////////////////3/v/////////////////////////+////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////////////////////////////////////////"
    "/////////////////////////////////////////////////////////////////////////////////////v//////////////////////"
    "///+9//////////////////////////32v39/////////////////////f3adP/7/f/////////////////9+/90BKb/////////////////"
    "/////6YEAQBy0vP+/////////////fPScgAB",
    /* Settings <- uploaded icon 3 (silver mechanical gear). */
    "//8AAL733/++9573fe++9/////////////++933vnve+99//vvcAAP////997///vvd97573//8859e905xxjNOc17085///nvd97773//99"
    "7///Xe///13vXe++9xznz3umMeMYYQiCEGEI4ximMc97HOe+913vfe///33vXe+e9zznvvf3vaYxBCGCEOc5Va2mMVWt5zmiEAQhpjH3vb73"
    "POee913vPOc85573170EIQAAbWsYxve9fe84xn3v173XvW1rAAAEIde9nvc85zznHOc859veZSnnOetada15zqpS4xjjGMMYjnP//zSlDGPn"
    "OWUp29485xzn+959765zIADHOZ7305xBCKYxEISSlG1rcYzLWlGMvvfHOSAArnN97/veHOea1kUppjEQhDSlYQgMY5a1TWvnOXGMPOeqUmEI"
    "NKUQhKYxRSma1hznPOc0pWEIilIc5yhChjE0pQQhghDnOZKUqlLznGUpKEIc54pSYQg0pTznPOcwhGEIZSlVrSQhrnMMY8MY05xVrZa1YQgM"
    "Y21rwxg0pWUpYQgwhDznHOfPe2EI85w4xnGMVa3TnFWt85wAALKUCELHOa5zohB1rRSlYQjPexzn+94QhEEIZSnznAQhLGOKUsMYcYzTnBSl"
    "YQiqUutawxjTnGUpQQgQhPve297znEEIKEL3vcc5JCHPe8MYohDnOTCECEKucyQhxzn3vQhCQQjznNveec4YxgQhZSlNaxCEQQgIQo5zCEIk"
    "ISxjtrWmMUEIEIRNa2UpBCEYxnnOGMaa1ixjAABFKXWtTWsgAAQhaUrLWghCbWsoQgxjda0kIQAALGOa1hjGGMY4xte9BCFFKShCrnMwhKYx"
    "ohCCEIIQKEKWtY5zKEJFKQQh1704xhjGGMb3vVnO05yCEAAAKELve65zcYzPe3GMrnOucyhCAACCENOcWc73vRjG970Yxte9OMaynAQhYQhB"
    "CCQhLGMEISxjJCFBCGEIBCGylDjG170Yxve9171Zzte9trUYxna1y1rjGEEIAAAAAAAAQQjjGMtadrUYxra9171Zzte99722tTjG1722tde9"
    "GMZ1rVGMbWssY21rUYx1rRjG1722tde9OMa2tfe9AAAAALa9t722tZa1lrW2tfe9+MX4xfjF9722tXa1lrW2tbe9trUAAAAAAQBz0vT9////"
    "/////////fTScwABBKb//////////////////////6YEdP/7/f/////////////////9+/9z2v39/////////////////////f3a9///////"
    "///////////////////3/v/////////////////////////+////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////////////////////////////////////////"
    "/////////////////////////////////////////////////////////////////////////////////////v//////////////////////"
    "///+9//////////////////////////32v39/////////////////////f3adP/7/f/////////////////9+/90BKb/////////////////"
    "/////6YEAQBy0vP+/////////////fPScgAB"
};

static LV_ATTRIBUTE_MEM_ALIGN uint8_t app_pixels[APP_COUNT][APP_BYTES];
static lv_image_dsc_t app_images[APP_COUNT];
static bool app_ready[APP_COUNT];

static int base64_digit(unsigned char c)
{
    if(c >= 'A' && c <= 'Z') return c - 'A';
    if(c >= 'a' && c <= 'z') return c - 'a' + 26;
    if(c >= '0' && c <= '9') return c - '0' + 52;
    if(c == '+') return 62;
    if(c == '/') return 63;
    return -1;
}

static bool decode_icon(const char *source, uint8_t *out, size_t count)
{
    uint32_t bits = 0;
    int pending = 0;
    size_t size = 0;

    while(*source != 0 && *source != '=') {
        int digit = base64_digit((unsigned char)*source++);

        if(digit < 0)
            return false;
        bits = (bits << 6) | (uint32_t)digit;
        pending += 6;
        if(pending >= 8) {
            pending -= 8;
            if(size >= count)
                return false;
            out[size++] = (uint8_t)(bits >> pending);
        }
    }
    return size == count;
}

const lv_image_dsc_t *crazypod_iphone_app_icon(
    enum crazypod_home_destination destination)
{
    lv_image_dsc_t *image;

    if(destination < CRAZYPOD_HOME_NOW_PLAYING ||
       destination >= CRAZYPOD_HOME_DESTINATION_COUNT)
        return NULL;
    if(app_ready[destination])
        return &app_images[destination];
    if(!decode_icon(app_base64[destination], app_pixels[destination],
                    APP_BYTES))
        return NULL;

    image = &app_images[destination];
    memset(image, 0, sizeof(*image));
    image->header.magic = LV_IMAGE_HEADER_MAGIC;
    image->header.cf = LV_COLOR_FORMAT_RGB565A8;
    image->header.w = APP_SIDE;
    image->header.h = APP_SIDE;
    image->header.stride = APP_SIDE * 2;
    image->data_size = APP_BYTES;
    image->data = app_pixels[destination];
    app_ready[destination] = true;
    return image;
}

#endif
