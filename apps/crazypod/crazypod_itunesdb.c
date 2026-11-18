#include "crazypod_itunesdb.h"

#include <string.h>

#define ITDB_MHOD_STRING_HEADER 40u
#define ITDB_READ_CHUNK 128u

static uint32_t little32(const uint8_t *data)
{
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8) |
        ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
}

static bool bounded_read(
    const struct crazypod_itunesdb_reader *reader, uint32_t offset,
    void *data, size_t count)
{
    return reader != NULL && reader->read_at != NULL &&
        offset <= reader->file_size &&
        count <= reader->file_size - offset &&
        reader->read_at(reader->context, offset, data, count);
}

static bool chunk_at(
    const struct crazypod_itunesdb_reader *reader,
    uint32_t offset, uint32_t limit, const char *type,
    uint32_t minimum_header, uint32_t *header, uint32_t *total)
{
    uint8_t data[16];

    if(offset > limit || limit - offset < sizeof(data) ||
       !bounded_read(reader, offset, data, sizeof(data)) ||
       memcmp(data, type, 4) != 0)
        return false;
    *header = little32(data + 4);
    *total = little32(data + 8);
    return *header >= minimum_header && *total >= *header &&
        *total <= limit - offset;
}

static void append_utf8(char *out, size_t capacity, size_t *length,
                        uint32_t codepoint)
{
    uint8_t bytes[4];
    unsigned count;
    unsigned index;

    if(codepoint < 0x80u) {
        bytes[0] = (uint8_t)codepoint;
        count = 1;
    }
    else if(codepoint < 0x800u) {
        bytes[0] = 0xc0u | (codepoint >> 6);
        bytes[1] = 0x80u | (codepoint & 63);
        count = 2;
    }
    else if(codepoint < 0x10000u) {
        bytes[0] = 0xe0u | (codepoint >> 12);
        bytes[1] = 0x80u | ((codepoint >> 6) & 63);
        bytes[2] = 0x80u | (codepoint & 63);
        count = 3;
    }
    else {
        bytes[0] = 0xf0u | (codepoint >> 18);
        bytes[1] = 0x80u | ((codepoint >> 12) & 63);
        bytes[2] = 0x80u | ((codepoint >> 6) & 63);
        bytes[3] = 0x80u | (codepoint & 63);
        count = 4;
    }
    if(*length + count >= capacity)
        return;
    for(index = 0; index < count; ++index)
        out[(*length)++] = (char)bytes[index];
    out[*length] = '\0';
}

static bool read_utf16(
    const struct crazypod_itunesdb_reader *reader, uint32_t offset,
    uint32_t byte_count, char *output, size_t capacity)
{
    uint8_t input[ITDB_READ_CHUNK];
    uint32_t position = 0;
    uint32_t high_surrogate = 0;
    size_t length = 0;

    if(capacity == 0 || (byte_count & 1u) != 0)
        return false;
    output[0] = '\0';
    while(position < byte_count) {
        uint32_t take = byte_count - position;
        uint32_t i;

        if(take > sizeof(input))
            take = sizeof(input);
        if(!bounded_read(reader, offset + position, input, take))
            return false;
        for(i = 0; i < take; i += 2) {
            uint32_t value = input[i] | ((uint32_t)input[i + 1] << 8);
            uint32_t codepoint;

            if(value == 0)
                return true;
            if(high_surrogate != 0) {
                if(value >= 0xdc00u && value <= 0xdfffu) {
                    codepoint = 0x10000u +
                        ((high_surrogate - 0xd800u) << 10) +
                        (value - 0xdc00u);
                    high_surrogate = 0;
                    append_utf8(output, capacity, &length, codepoint);
                    continue;
                }
                append_utf8(output, capacity, &length, 0xfffdu);
                high_surrogate = 0;
            }
            if(value >= 0xd800u && value <= 0xdbffu) {
                high_surrogate = value;
                continue;
            }
            codepoint = value >= 0xdc00u && value <= 0xdfffu
                ? 0xfffdu : value;
            if(codepoint != 0xfeffu)
                append_utf8(output, capacity, &length, codepoint);
        }
        position += take;
        if(length + 4 >= capacity)
            break;
    }
    if(high_surrogate != 0)
        append_utf8(output, capacity, &length, 0xfffdu);
    return true;
}

static bool read_mhod(
    const struct crazypod_itunesdb_reader *reader,
    uint32_t offset, uint32_t total, char *output, size_t capacity)
{
    uint8_t data[ITDB_MHOD_STRING_HEADER];
    uint32_t text_size;

    if(total < sizeof(data) ||
       !bounded_read(reader, offset, data, sizeof(data)))
        return false;
    /* Type 1 is UTF-16LE; unsupported encodings leave tags unchanged. */
    if(little32(data + 24) != 1)
        return true;
    text_size = little32(data + 28);
    if(text_size > total - sizeof(data))
        return false;
    return read_utf16(reader, offset + sizeof(data),
                      text_size, output, capacity);
}

static void normalize_path(char *path)
{
    char *cursor;

    if(path[0] != ':')
        return;
    for(cursor = path; *cursor != '\0'; ++cursor)
        if(*cursor == ':')
            *cursor = '/';
}

static bool valid_ipod_music_path(const char *path)
{
    static const char prefix[] = "/iPod_Control/Music/";
    size_t i;

    for(i = 0; i < sizeof(prefix) - 1; ++i) {
        unsigned char a = (unsigned char)path[i];
        unsigned char b = (unsigned char)prefix[i];

        if(a >= 'A' && a <= 'Z')
            a += 'a' - 'A';
        if(b >= 'A' && b <= 'Z')
            b += 'a' - 'A';
        if(a == '\0' || a != b)
            return false;
    }
    /* Compare only normal relative path components. A malformed MHOD
     * must not be able to select a file outside the original music tree. */
    if(path[i] == '\0')
        return false;
    while(path[i] != '\0') {
        size_t start = i;

        while(path[i] != '\0' && path[i] != '/') {
            if(path[i] == ':' || path[i] == '\\')
                return false;
            ++i;
        }
        if(i == start || (i - start == 1 && path[start] == '.') ||
           (i - start == 2 && path[start] == '.' &&
            path[start + 1] == '.'))
            return false;
        if(path[i] == '/') {
            ++i;
            if(path[i] == '\0')
                return false;
        }
    }
    return true;
}

static bool visit_track(
    const struct crazypod_itunesdb_reader *reader,
    const struct crazypod_itunesdb_callbacks *callbacks,
    uint32_t start, uint32_t header, uint32_t total)
{
    struct crazypod_itunesdb_track track = {0};
    uint8_t data[20];
    uint32_t cursor = start + header;
    uint32_t end = start + total;
    uint32_t child_count;
    uint32_t child;

    if(header < sizeof(data) ||
       !bounded_read(reader, start, data, sizeof(data)))
        return false;
    child_count = little32(data + 12);
    if(header >= 0x38u) {
        uint8_t fields[12];

        if(!bounded_read(reader, start + 0x2cu, fields,
                         sizeof(fields)))
            return false;
        track.track_number = little32(fields);
        track.year = little32(fields + 8);
    }
    if(header >= 0x64u) {
        uint8_t disc[4];

        if(!bounded_read(reader, start + 0x60u, disc,
                         sizeof(disc)))
            return false;
        track.disc_number = little32(disc);
    }
    for(child = 0; child < child_count; ++child) {
        uint8_t object[16];
        uint32_t object_header;
        uint32_t object_total;
        uint32_t type;
        char *destination = NULL;
        size_t capacity = 0;

        if(cursor > end || end - cursor < sizeof(object) ||
           !chunk_at(reader, cursor, end, "mhod", 24u,
                     &object_header, &object_total) ||
           !bounded_read(reader, cursor, object, sizeof(object)))
            return false;
        type = little32(object + 12);
        switch(type) {
        case 1:
            destination = track.title;
            capacity = sizeof(track.title);
            break;
        case 2:
            destination = track.path;
            capacity = sizeof(track.path);
            break;
        case 3:
            destination = track.album;
            capacity = sizeof(track.album);
            break;
        case 4:
            destination = track.artist;
            capacity = sizeof(track.artist);
            break;
        case 22:
            destination = track.album_artist;
            capacity = sizeof(track.album_artist);
            break;
        default:
            break;
        }
        if(destination != NULL &&
           !read_mhod(reader, cursor, object_total,
                      destination, capacity))
            return false;
        cursor += object_total;
    }
    normalize_path(track.path);
    if(!valid_ipod_music_path(track.path))
        return true;
    return callbacks->on_track(callbacks->context, &track);
}

static bool visit_track_section(
    const struct crazypod_itunesdb_reader *reader,
    const struct crazypod_itunesdb_callbacks *callbacks,
    uint32_t position, uint32_t section_end)
{
    uint8_t list[12];
    uint32_t position_in_list;
    uint32_t track_count;
    uint32_t i;

    if(position > section_end || section_end - position < sizeof(list) ||
       !bounded_read(reader, position, list, sizeof(list)) ||
       memcmp(list, "mhlt", 4) != 0)
        return false;
    position_in_list = little32(list + 4);
    track_count = little32(list + 8);
    if(position_in_list < sizeof(list) ||
       position_in_list > section_end - position)
        return false;
    position += position_in_list;
    for(i = 0; i < track_count; ++i) {
        uint32_t header;
        uint32_t total;

        if(callbacks->cancelled != NULL &&
           callbacks->cancelled(callbacks->context))
            return false;
        if(!chunk_at(reader, position, section_end, "mhit", 20u,
                     &header, &total) ||
           !visit_track(reader, callbacks, position, header, total))
            return false;
        position += total;
    }
    return true;
}

bool crazypod_itunesdb_visit(
    const struct crazypod_itunesdb_reader *reader,
    const struct crazypod_itunesdb_callbacks *callbacks)
{
    uint32_t root_header;
    uint32_t root_total;
    uint32_t cursor;

    if(reader == NULL || callbacks == NULL ||
       callbacks->on_track == NULL ||
       !chunk_at(reader, 0u, reader->file_size, "mhbd", 12u,
                 &root_header, &root_total))
        return false;
    cursor = root_header;
    while(cursor < root_total) {
        uint32_t section_header;
        uint32_t section_total;
        uint8_t data[16];

        if(callbacks->cancelled != NULL &&
           callbacks->cancelled(callbacks->context))
            return false;
        if(!chunk_at(reader, cursor, root_total, "mhsd", 16u,
                     &section_header, &section_total) ||
           !bounded_read(reader, cursor, data, sizeof(data)))
            return false;
        if(little32(data + 12) == 1u)
            return visit_track_section(
                reader, callbacks, cursor + section_header,
                cursor + section_total);
        cursor += section_total;
    }
    return false;
}
