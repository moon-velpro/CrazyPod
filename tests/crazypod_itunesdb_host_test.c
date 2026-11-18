#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "crazypod_itunesdb.h"

/* Minimal in-memory classic iTunesDB with a skipped album section. */
static uint8_t sample[4096];
static uint32_t used;
static int matches;
static struct crazypod_itunesdb_track last_track;

static void put32(uint32_t at, uint32_t value)
{
    sample[at] = (uint8_t)value;
    sample[at + 1] = (uint8_t)(value >> 8);
    sample[at + 2] = (uint8_t)(value >> 16);
    sample[at + 3] = (uint8_t)(value >> 24);
}

static void tag(uint32_t at, const char *name)
{
    memcpy(sample + at, name, 4);
}

static uint32_t string_object(uint32_t type, const uint16_t *text,
                              uint32_t codepoints)
{
    uint32_t start = used;
    uint32_t i;

    tag(start, "mhod");
    put32(start + 4, 24);
    put32(start + 8, 40 + codepoints * 2);
    put32(start + 12, type);
    put32(start + 24, 1); /* UTF16LE */
    put32(start + 28, codepoints * 2);
    for(i = 0; i < codepoints; ++i) {
        sample[start + 40 + i * 2] = (uint8_t)text[i];
        sample[start + 41 + i * 2] = (uint8_t)(text[i] >> 8);
    }
    used += 40 + codepoints * 2;
    return start;
}

static void ascii_object(uint32_t type, const char *text)
{
    uint16_t utf16[256];
    uint32_t i;
    uint32_t length = (uint32_t)strlen(text);

    assert(length <= 256);
    for(i = 0; i < length; ++i)
        utf16[i] = (uint8_t)text[i];
    (void)string_object(type, utf16, length);
}

static void build_sample(void)
{
    static const uint16_t title[] = { 0x4e2d, 0x6587, 0x6b4c };
    static const uint16_t artist[] = {
        'A', 'r', 't', 'i', 's', 't', ' ', 0xd83c, 0xdfb5
    };
    uint32_t section;
    uint32_t mhit;

    memset(sample, 0, sizeof(sample));
    tag(0, "mhbd");
    put32(4, 32);
    used = 32;
    /* A non-track mhsd section must be skipped. */
    tag(used, "mhsd");
    put32(used + 4, 16);
    put32(used + 8, 16);
    put32(used + 12, 4);
    used += 16;

    section = used;
    tag(section, "mhsd");
    put32(section + 4, 16);
    put32(section + 12, 1);
    used += 16;
    tag(used, "mhlt");
    put32(used + 4, 12);
    put32(used + 8, 1);
    used += 12;

    mhit = used;
    tag(mhit, "mhit");
    put32(mhit + 4, 104);
    put32(mhit + 12, 5);
    put32(mhit + 0x2c, 7);
    put32(mhit + 0x34, 2025);
    put32(mhit + 0x60, 2);
    used += 104;
    ascii_object(2, ":iPod_Control:Music:F00:ABCD.mp3");
    (void)string_object(1, title, sizeof(title) / sizeof(title[0]));
    (void)string_object(4, artist, sizeof(artist) / sizeof(artist[0]));
    ascii_object(3, "My Album");
    ascii_object(22, "Album Artist");
    put32(mhit + 8, used - mhit);
    put32(section + 8, used - section);
    put32(8, used);
}

static bool read_memory(void *context, uint32_t offset,
                        void *buffer, size_t count)
{
    (void)context;
    if(offset > used || count > used - offset)
        return false;
    memcpy(buffer, sample + offset, count);
    return true;
}

static bool on_track(void *context,
                     const struct crazypod_itunesdb_track *track)
{
    (void)context;
    ++matches;
    last_track = *track;
    return true;
}

static void test_classic_metadata(void)
{
    struct crazypod_itunesdb_reader reader;
    struct crazypod_itunesdb_callbacks callbacks = {
        .context = NULL,
        .on_track = on_track,
        .cancelled = NULL,
    };

    build_sample();
    reader.file_size = used;
    reader.context = NULL;
    reader.read_at = read_memory;
    matches = 0;
    assert(crazypod_itunesdb_visit(&reader, &callbacks));
    assert(matches == 1);
    assert(strcmp(last_track.path,
                  "/iPod_Control/Music/F00/ABCD.mp3") == 0);
    assert(strcmp(last_track.title, "\xe4\xb8\xad\xe6\x96\x87\xe6\xad\x8c") == 0);
    assert(strcmp(last_track.artist, "Artist \xf0\x9f\x8e\xb5") == 0);
    assert(strcmp(last_track.album, "My Album") == 0);
    assert(strcmp(last_track.album_artist, "Album Artist") == 0);
    assert(last_track.track_number == 7);
    assert(last_track.year == 2025);
    assert(last_track.disc_number == 2);
}

static void test_corruption_is_bounded(void)
{
    struct crazypod_itunesdb_reader reader = {
        .context = NULL, .read_at = read_memory
    };
    struct crazypod_itunesdb_callbacks callbacks = {
        .context = NULL, .on_track = on_track, .cancelled = NULL
    };

    build_sample();
    reader.file_size = used - 1;
    assert(!crazypod_itunesdb_visit(&reader, &callbacks));

    build_sample();
    put32(32 + 16 + 16 + 12 + 8, 0xffffffffu); /* mhit total */
    reader.file_size = used;
    assert(!crazypod_itunesdb_visit(&reader, &callbacks));

    build_sample();
    /* Guard against a corrupt child count walking beyond the record. */
    put32(32 + 16 + 16 + 12 + 12, 999999u);
    assert(!crazypod_itunesdb_visit(&reader, &callbacks));
}

static void test_path_guard(void)
{
    struct crazypod_itunesdb_reader reader;
    struct crazypod_itunesdb_callbacks callbacks = {
        .context = NULL, .on_track = on_track, .cancelled = NULL
    };
    uint32_t offset;

    build_sample();
    offset = 32 + 16 + 16 + 12 + 104 + 40;
    /* Make the first path byte invalid; it must never match a local track. */
    sample[offset] = '!';
    reader.file_size = used;
    reader.context = NULL;
    reader.read_at = read_memory;
    matches = 0;
    assert(crazypod_itunesdb_visit(&reader, &callbacks));
    assert(matches == 0);
}

int main(void)
{
    test_classic_metadata();
    test_corruption_is_bounded();
    test_path_guard();
    return 0;
}
