#include "config.h"

#ifdef IPOD_6G

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "file.h"
#include "system.h"

#include "crazypod_icons.h"

/*
 * Keep the original, bundled "basic" artwork as CrazyPod's one fixed
 * application icon set. Decode and downsample 160x160 BMP files only once
 * during UI startup; Home itself never opens files or scales images.
 */
#define SOURCE_SIZE 160
#define ICON_ROW_BYTES (SOURCE_SIZE * 4)
#define ICON_PIXELS (CRAZYPOD_ICON_SIZE * CRAZYPOD_ICON_SIZE * 4)
#define SAMPLE_SCALE (SOURCE_SIZE / CRAZYPOD_ICON_SIZE)

struct icon_slot {
    uint8_t pixels[ICON_PIXELS] CACHEALIGN_AT_LEAST_ATTR(16);
    struct crazypod_icon image;
    bool valid;
};

static struct icon_slot slots[CRAZYPOD_ICON_COUNT];

static const char *const app_paths[CRAZYPOD_ICON_COUNT] = {
    "music", "podcasts", "mini_apps", "shuffle",
    "screen_lock", "photos", "diy", "fitness", "books", "notes",
    "clock", "contacts", "calendar", "stopwatch",
    "extras", "settings"
};

static uint16_t read_le16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t read_le32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static bool read_exact(int fd, void *buffer, size_t size)
{
    uint8_t *cursor = buffer;

    while(size != 0) {
        ssize_t n = read(fd, cursor, size);

        if(n <= 0)
            return false;
        cursor += n;
        size -= (size_t)n;
    }
    return true;
}

static bool load_icon(struct icon_slot *slot, const char *path)
{
    uint8_t header[54];
    uint8_t row[ICON_ROW_BYTES];
    uint32_t sums[CRAZYPOD_ICON_SIZE][4];
    uint32_t offset;
    int32_t height;
    bool top_down;
    int file;
    int block;

    file = open(path, O_RDONLY);
    if(file < 0)
        return false;
    if(!read_exact(file, header, sizeof(header)) ||
       header[0] != 'B' || header[1] != 'M' ||
       read_le32(header + 18) != SOURCE_SIZE ||
       (read_le32(header + 22) != SOURCE_SIZE &&
        read_le32(header + 22) != (uint32_t)(-SOURCE_SIZE)) ||
       read_le16(header + 26) != 1 ||
       read_le16(header + 28) != 32 ||
       read_le32(header + 30) != 0) {
        close(file);
        return false;
    }
    offset = read_le32(header + 10);
    height = (int32_t)read_le32(header + 22);
    top_down = height < 0;
    if(lseek(file, (off_t)offset, SEEK_SET) < 0) {
        close(file);
        return false;
    }

    /*
     * BMP rows are bottom-up unless height is negative. Reading in disk
     * order makes each 8x8 box-filter block sequential, without seeks.
     * RGB is averaged with alpha weighting so transparent edge pixels
     * do not create dark fringes when composited by LVGL.
     */
    for(block = 0; block < CRAZYPOD_ICON_SIZE; ++block) {
        int dest_y = top_down
            ? block : CRAZYPOD_ICON_SIZE - 1 - block;
        int y;
        int x;

        memset(sums, 0, sizeof(sums));
        for(y = 0; y < SAMPLE_SCALE; ++y) {
            if(!read_exact(file, row, sizeof(row))) {
                close(file);
                return false;
            }
            for(x = 0; x < SOURCE_SIZE; ++x) {
                const uint8_t *src = &row[x * 4];
                uint32_t alpha = src[3];
                uint32_t *sum = sums[x / SAMPLE_SCALE];

                sum[0] += src[0] * alpha;
                sum[1] += src[1] * alpha;
                sum[2] += src[2] * alpha;
                sum[3] += alpha;
            }
        }
        for(x = 0; x < CRAZYPOD_ICON_SIZE; ++x) {
            uint8_t *dst = &slot->pixels[
                (dest_y * CRAZYPOD_ICON_SIZE + x) * 4];
            const uint32_t *sum = sums[x];
            uint32_t alpha = sum[3];

            dst[0] = alpha ? sum[0] / alpha : 0;
            dst[1] = alpha ? sum[1] / alpha : 0;
            dst[2] = alpha ? sum[2] / alpha : 0;
            dst[3] = alpha / (SAMPLE_SCALE * SAMPLE_SCALE);
        }
    }
    close(file);
    slot->image.pixels = slot->pixels;
    slot->image.width = CRAZYPOD_ICON_SIZE;
    slot->image.height = CRAZYPOD_ICON_SIZE;
    slot->image.stride = CRAZYPOD_ICON_SIZE * 4;
    slot->valid = true;
    return true;
}

void crazypod_icons_init(void)
{
    char path[MAX_PATH];
    int index;

    memset(slots, 0, sizeof(slots));
    for(index = 0; index < CRAZYPOD_ICON_COUNT; ++index) {
        snprintf(path, sizeof(path),
                 "/.rockbox/crazypod/icons/basic/%s.bmp",
                 app_paths[index]);
        (void)load_icon(&slots[index], path);
    }
}

const struct crazypod_icon *crazypod_icon_get(int index)
{
    if(index < 0 || index >= CRAZYPOD_ICON_COUNT ||
       !slots[index].valid)
        return NULL;
    return &slots[index].image;
}

#endif
