#include <assert.h>

#include "../apps/crazypod/ui/features/music/crazypod_music_root_menu_model.h"

int main(void)
{
    int index;
    unsigned int seen = 0;

    assert(CRAZYPOD_MUSIC_ROOT_COUNT == 7);
    assert(crazypod_music_root_legacy_index(
        CRAZYPOD_MUSIC_ROOT_ALBUMS) == 5);
    assert(crazypod_music_root_legacy_index(
        CRAZYPOD_MUSIC_ROOT_ARTISTS) == 4);
    assert(crazypod_music_root_legacy_index(
        CRAZYPOD_MUSIC_ROOT_SONGS) == 6);
    assert(crazypod_music_root_legacy_index(
        CRAZYPOD_MUSIC_ROOT_PLAYLISTS) == 3);
    assert(crazypod_music_root_legacy_index(
        CRAZYPOD_MUSIC_ROOT_COVER_FLOW) == 1);
    assert(crazypod_music_root_legacy_index(
        CRAZYPOD_MUSIC_ROOT_SEARCH) == 7);
    assert(crazypod_music_root_legacy_index(-1) == -1);
    assert(crazypod_music_root_legacy_index(
        CRAZYPOD_MUSIC_ROOT_SHUFFLE) == -1);
    assert(crazypod_music_root_legacy_index(7) == -1);
    assert(crazypod_music_root_legacy_index(100) == -1);

    for(index = 0; index < CRAZYPOD_MUSIC_ROOT_COUNT; ++index) {
        int route = crazypod_music_root_legacy_index(index);

        if(index == CRAZYPOD_MUSIC_ROOT_SHUFFLE) {
            assert(route == -1); /* A playback command, not a route. */
            continue;
        }
        assert(route >= 1 && route <= 7);
        assert(route != 2); /* All Music duplicates Songs. */
        assert((seen & (1u << route)) == 0);
        seen |= (1u << route);
    }
    return 0;
}
