#include "config.h"

#ifdef IPOD_6G

#include "../features/music/crazypod_music_feature.h"
#include "../features/now_playing/crazypod_now_playing_feature.h"
#include "../features/settings/crazypod_settings_feature.h"
#include "../shell/crazypod_home_actions.h"
#include "crazypod_route_query.h"
#include "crazypod_route_registry.h"

/* Navigation reads the music product's feature facades only. Retired
 * applications have neither route metadata nor query dispatch here. */
int crazypod_route_query_item_count(
    const struct route_state *state, const char *music_search_query)
{
    const struct crazypod_feature *feature;

    if(state == NULL)
        return 0;
    if(crazypod_home_actions_is_route(state->route))
        return crazypod_home_actions_item_count(state->route);
    feature = crazypod_route_registry_feature(state->route);
    if(feature == NULL)
        return 0;
    switch(feature->id) {
    case CRAZYPOD_FEATURE_MUSIC:
        return crazypod_music_feature_item_count(
            state, music_search_query);
    case CRAZYPOD_FEATURE_NOW_PLAYING:
        return crazypod_now_playing_feature_item_count(state);
    case CRAZYPOD_FEATURE_SETTINGS:
        return crazypod_settings_feature_item_count(state);
    default:
        return 0;
    }
}

const char *crazypod_route_query_item_title(
    const struct route_state *state, int index,
    const char *music_search_query)
{
    const struct crazypod_feature *feature;
    const char *title = "";
    bool handled = false;

    if(state == NULL)
        return "";
    if(crazypod_home_actions_is_route(state->route))
        return crazypod_home_actions_item_title(index);
    feature = crazypod_route_registry_feature(state->route);
    if(feature == NULL)
        return "";
    switch(feature->id) {
    case CRAZYPOD_FEATURE_MUSIC:
        handled = crazypod_music_feature_item_title(
            state, index, music_search_query, &title);
        break;
    case CRAZYPOD_FEATURE_NOW_PLAYING:
        handled = crazypod_now_playing_feature_item_title(
            state, index, &title);
        break;
    case CRAZYPOD_FEATURE_SETTINGS:
        handled = crazypod_settings_feature_item_title(
            state, index, &title);
        break;
    default:
        break;
    }
    return handled ? title : "";
}

enum crazypod_menu_icon crazypod_route_query_item_icon(
    const struct route_state *state, int index)
{
    const struct crazypod_feature *feature;

    if(state == NULL || index < 0 ||
       crazypod_home_actions_is_route(state->route))
        return CRAZYPOD_MENU_ICON_NONE;
    feature = crazypod_route_registry_feature(state->route);
    if(feature == NULL)
        return CRAZYPOD_MENU_ICON_NONE;
    switch(feature->id) {
    case CRAZYPOD_FEATURE_MUSIC:
        return crazypod_music_feature_item_icon(state, index);
    case CRAZYPOD_FEATURE_NOW_PLAYING:
        return crazypod_now_playing_feature_item_icon(state, index);
    case CRAZYPOD_FEATURE_SETTINGS:
        return crazypod_settings_feature_item_icon(state, index);
    default:
        return CRAZYPOD_MENU_ICON_NONE;
    }
}

const char *crazypod_route_query_title(
    const struct route_state *state)
{
    const struct crazypod_feature *feature;

    if(state == NULL)
        return "";
    if(crazypod_home_actions_is_route(state->route))
        return crazypod_home_actions_title(state->route);
    feature = crazypod_route_registry_feature(state->route);
    if(feature == NULL)
        return "";
    switch(feature->id) {
    case CRAZYPOD_FEATURE_MUSIC:
        return crazypod_music_feature_title(state);
    case CRAZYPOD_FEATURE_NOW_PLAYING:
        return crazypod_now_playing_feature_title(state);
    case CRAZYPOD_FEATURE_SETTINGS:
        return crazypod_settings_feature_title(state);
    default:
        return "";
    }
}

bool crazypod_route_query_item_is_current(
    const struct route_state *state, int index)
{
    const struct crazypod_feature *feature;

    if(state == NULL)
        return false;
    feature = crazypod_route_registry_feature(state->route);
    return feature != NULL &&
           feature->id == CRAZYPOD_FEATURE_SETTINGS &&
           crazypod_settings_feature_item_is_current(state, index);
}

#endif
