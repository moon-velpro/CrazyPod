#ifndef CRAZYPOD_ICONS_H
#define CRAZYPOD_ICONS_H

#include <stdint.h>

#define CRAZYPOD_ICON_COUNT 17
#define CRAZYPOD_ICON_SIZE 20

struct crazypod_icon {
    const uint8_t *pixels;
    int width;
    int height;
    int stride;
};

void crazypod_icons_init(void);
const struct crazypod_icon *crazypod_icon_get(int index);

#endif
