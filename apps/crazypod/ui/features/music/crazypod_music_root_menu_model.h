#ifndef CRAZYPOD_MUSIC_ROOT_MENU_MODEL_H
#define CRAZYPOD_MUSIC_ROOT_MENU_MODEL_H

/*
 * Clean-slate Music library navigation.
 *
 * Old route IDs and activation implementation remain unchanged. Only the
 * visible Music menu is remapped. Now Playing is a Home destination, while
 * All Music duplicates Songs, so neither belongs in the library root.
 *
 * Public UI order: Albums, Artists, Songs, Playlists, Cover Flow, Search.
 * The mapping below is the only compatibility bridge to legacy indices.
 */
enum crazypod_music_root_item {
    CRAZYPOD_MUSIC_ROOT_ALBUMS = 0,
    CRAZYPOD_MUSIC_ROOT_ARTISTS,
    CRAZYPOD_MUSIC_ROOT_SONGS,
    CRAZYPOD_MUSIC_ROOT_PLAYLISTS,
    CRAZYPOD_MUSIC_ROOT_COVER_FLOW,
    CRAZYPOD_MUSIC_ROOT_SEARCH,
    CRAZYPOD_MUSIC_ROOT_COUNT
};

static inline int crazypod_music_root_legacy_index(int selected)
{
    static const unsigned char legacy[] = { 5, 4, 6, 3, 1, 7 };

    if(selected < 0 || selected >= CRAZYPOD_MUSIC_ROOT_COUNT)
        return -1;
    return legacy[selected];
}

#endif
