#include <stddef.h>

#include "crazypod_route_registry.h"

#define FEATURE_ROUTE(route_, feature_) \
    [route_] = { \
        route_, CRAZYPOD_ROUTE_OWNER_FEATURE, \
        &features[feature_], CRAZYPOD_ROUTE_FLAG_NONE \
    }
#define FEATURE_ROUTE_FLAGS(route_, feature_, flags_) \
    [route_] = { \
        route_, CRAZYPOD_ROUTE_OWNER_FEATURE, \
        &features[feature_], flags_ \
    }
#define SHELL_ROUTE(route_) \
    [route_] = { \
        route_, CRAZYPOD_ROUTE_OWNER_SHELL, NULL, \
        CRAZYPOD_ROUTE_FLAG_NONE \
    }

/* Runtime feature registrations. Historic enum IDs remain allocated in
 * crazypod_feature.h for existing settings/state, not for navigation. */
static const struct crazypod_feature features[CRAZYPOD_FEATURE_COUNT] = {
    [CRAZYPOD_FEATURE_MUSIC] = {
        CRAZYPOD_FEATURE_MUSIC, "music"
    },
    [CRAZYPOD_FEATURE_NOW_PLAYING] = {
        CRAZYPOD_FEATURE_NOW_PLAYING, "now_playing"
    },
    [CRAZYPOD_FEATURE_SETTINGS] = {
        CRAZYPOD_FEATURE_SETTINGS, "settings"
    },
};

const struct crazypod_feature *crazypod_feature_get(
    enum crazypod_feature_id id)
{
    if(id != CRAZYPOD_FEATURE_MUSIC &&
       id != CRAZYPOD_FEATURE_NOW_PLAYING &&
       id != CRAZYPOD_FEATURE_SETTINGS)
        return NULL;
    return &features[id];
}

/* Deliberately sparse: removed products have no registered descriptor.
 * This avoids compiling their routing metadata into production firmware.
 * Numeric slots are left intact solely for old persisted route values. */
static const struct crazypod_route_descriptor
routes[CRAZYPOD_ROUTE_COUNT] = {
    FEATURE_ROUTE_FLAGS(
        MUSIC_ROUTE_MENU, CRAZYPOD_FEATURE_MUSIC,
        CRAZYPOD_ROUTE_FLAG_PREVIEW |
        CRAZYPOD_ROUTE_FLAG_SKEUOMORPHIC_PREVIEW),
    FEATURE_ROUTE_FLAGS(
        MUSIC_ROUTE_ALBUM_FLOW, CRAZYPOD_FEATURE_MUSIC,
        CRAZYPOD_ROUTE_FLAG_SOLID_BLACK),
    FEATURE_ROUTE_FLAGS(
        MUSIC_ROUTE_ALL, CRAZYPOD_FEATURE_MUSIC,
        CRAZYPOD_ROUTE_FLAG_PREVIEW),
    FEATURE_ROUTE_FLAGS(
        MUSIC_ROUTE_PLAYLISTS, CRAZYPOD_FEATURE_MUSIC,
        CRAZYPOD_ROUTE_FLAG_PREVIEW),
    FEATURE_ROUTE_FLAGS(
        MUSIC_ROUTE_PLAYLIST_SONGS, CRAZYPOD_FEATURE_MUSIC,
        CRAZYPOD_ROUTE_FLAG_PREVIEW),
    FEATURE_ROUTE_FLAGS(
        MUSIC_ROUTE_ARTISTS, CRAZYPOD_FEATURE_MUSIC,
        CRAZYPOD_ROUTE_FLAG_PREVIEW),
    FEATURE_ROUTE_FLAGS(
        MUSIC_ROUTE_ARTIST_SONGS, CRAZYPOD_FEATURE_MUSIC,
        CRAZYPOD_ROUTE_FLAG_PREVIEW),
    FEATURE_ROUTE_FLAGS(
        MUSIC_ROUTE_ALBUMS, CRAZYPOD_FEATURE_MUSIC,
        CRAZYPOD_ROUTE_FLAG_PREVIEW),
    FEATURE_ROUTE_FLAGS(
        MUSIC_ROUTE_ALBUM_SONGS, CRAZYPOD_FEATURE_MUSIC,
        CRAZYPOD_ROUTE_FLAG_PREVIEW),
    FEATURE_ROUTE_FLAGS(
        MUSIC_ROUTE_SONGS, CRAZYPOD_FEATURE_MUSIC,
        CRAZYPOD_ROUTE_FLAG_PREVIEW),
    FEATURE_ROUTE(MUSIC_ROUTE_SEARCH, CRAZYPOD_FEATURE_MUSIC),
    FEATURE_ROUTE_FLAGS(
        MUSIC_ROUTE_SEARCH_RESULTS, CRAZYPOD_FEATURE_MUSIC,
        CRAZYPOD_ROUTE_FLAG_PREVIEW),
    FEATURE_ROUTE_FLAGS(
        MUSIC_ROUTE_QUEUE, CRAZYPOD_FEATURE_NOW_PLAYING,
        CRAZYPOD_ROUTE_FLAG_PREVIEW),
    FEATURE_ROUTE(
        MUSIC_ROUTE_NOW_PLAYING, CRAZYPOD_FEATURE_NOW_PLAYING),
    FEATURE_ROUTE(SETTINGS_ROUTE_MENU, CRAZYPOD_FEATURE_SETTINGS),
    FEATURE_ROUTE(
        SETTINGS_ROUTE_VALUE_CHOICES, CRAZYPOD_FEATURE_SETTINGS),
    FEATURE_ROUTE(SETTINGS_ROUTE_SOUND, CRAZYPOD_FEATURE_SETTINGS),
    FEATURE_ROUTE(SETTINGS_ROUTE_EQ_STUDIO, CRAZYPOD_FEATURE_SETTINGS),
    FEATURE_ROUTE(SETTINGS_ROUTE_DISPLAY, CRAZYPOD_FEATURE_SETTINGS),
    FEATURE_ROUTE(SETTINGS_ROUTE_DATE_TIME, CRAZYPOD_FEATURE_SETTINGS),
    FEATURE_ROUTE(SETTINGS_ROUTE_PLAYBACK, CRAZYPOD_FEATURE_SETTINGS),
    FEATURE_ROUTE(SETTINGS_ROUTE_POWER, CRAZYPOD_FEATURE_SETTINGS),
    FEATURE_ROUTE(SETTINGS_ROUTE_CONTROLS, CRAZYPOD_FEATURE_SETTINGS),
    FEATURE_ROUTE(
        SETTINGS_ROUTE_MAIN_MENU, CRAZYPOD_FEATURE_SETTINGS),
    FEATURE_ROUTE(
        SETTINGS_ROUTE_MAIN_MENU_ACTIONS, CRAZYPOD_FEATURE_SETTINGS),
    FEATURE_ROUTE(PODCASTS_ROUTE_MENU, CRAZYPOD_FEATURE_MUSIC),
    SHELL_ROUTE(HOME_ROUTE_ACTIONS),
    SHELL_ROUTE(HOME_ROUTE_BRIGHTNESS),
    SHELL_ROUTE(HOME_ROUTE_VOLUME),
};

const struct crazypod_route_descriptor *
crazypod_route_registry_get(enum crazypod_route route)
{
    if(!crazypod_ui_route_supported(route))
        return NULL;
    return &routes[route];
}

const struct crazypod_feature *
crazypod_route_registry_feature(enum crazypod_route route)
{
    const struct crazypod_route_descriptor *descriptor =
        crazypod_route_registry_get(route);

    return descriptor != NULL &&
           descriptor->owner == CRAZYPOD_ROUTE_OWNER_FEATURE
        ? descriptor->feature : NULL;
}

bool crazypod_route_registry_is_shell(enum crazypod_route route)
{
    const struct crazypod_route_descriptor *descriptor =
        crazypod_route_registry_get(route);

    return descriptor != NULL &&
           descriptor->owner == CRAZYPOD_ROUTE_OWNER_SHELL;
}

bool crazypod_route_registry_has_flag(
    enum crazypod_route route, enum crazypod_route_flag flag)
{
    const struct crazypod_route_descriptor *descriptor =
        crazypod_route_registry_get(route);

    return descriptor != NULL &&
           (descriptor->flags & (unsigned int)flag) != 0;
}

bool crazypod_route_registry_validate(void)
{
    enum crazypod_route route;

    for(route = 0; route < CRAZYPOD_ROUTE_COUNT; ++route) {
        const struct crazypod_route_descriptor *descriptor =
            &routes[route];

        if(!crazypod_ui_route_supported(route)) {
            if(descriptor->feature != NULL ||
               descriptor->flags != CRAZYPOD_ROUTE_FLAG_NONE)
                return false;
            continue;
        }
        if(descriptor->route != route)
            return false;
        if(descriptor->owner == CRAZYPOD_ROUTE_OWNER_FEATURE) {
            if(descriptor->feature == NULL ||
               crazypod_feature_get(descriptor->feature->id) !=
                   descriptor->feature)
                return false;
        }
        else if(descriptor->owner != CRAZYPOD_ROUTE_OWNER_SHELL ||
                descriptor->feature != NULL)
            return false;
    }
    return true;
}
