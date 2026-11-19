#ifndef CRAZYPOD_ITUNESDB_H
#define CRAZYPOD_ITUNESDB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Read-only classic iPod database metadata, without full-file allocation. */
#define CRAZYPOD_ITUNESDB_PATH_SIZE 512

struct crazypod_itunesdb_track {
    char path[CRAZYPOD_ITUNESDB_PATH_SIZE];
    char title[96];
    char artist[72];
    char album[72];
    char album_artist[72];
    uint32_t year;
    uint32_t track_number;
    uint32_t disc_number;
};

struct crazypod_itunesdb_reader {
    uint32_t file_size;
    void *context;
    bool (*read_at)(void *context, uint32_t offset,
                    void *buffer, size_t count);
};

struct crazypod_itunesdb_callbacks {
    void *context;
    bool (*on_track)(void *context,
                     const struct crazypod_itunesdb_track *track);
    bool (*cancelled)(void *context);
};

/* An invalid database must never block normal filesystem tag scanning. */
bool crazypod_itunesdb_visit(
    const struct crazypod_itunesdb_reader *reader,
    const struct crazypod_itunesdb_callbacks *callbacks);

#endif
