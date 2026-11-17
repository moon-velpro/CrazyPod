#include "config.h"
#include "../../../crazypod_l10n.h"

#ifdef IPOD_6G

#include "../../../crazypod_artwork.h"
#include "crazypod_now_playing_feature.h"

/*
 * The music-only product has exactly one built-in Now Playing renderer.
 * Legacy theme selections are intentionally left on the user's disk,
 * but are never read, loaded, executed or rewritten by this edition.
 *
 * All public callers retain a well-defined API, allowing the old
 * MiniApp routes to be removed separately without changing Rockbox,
 * audio/album artwork ownership, or existing persistent app IDs.
 */
void crazypod_now_playing_theme_prepare(void)
{
    crazypod_now_playing_artwork_set_source_size(
        CRAZYPOD_ARTWORK_CACHE_SIZE);
}

int crazypod_now_playing_theme_choice_count(void)
{
    return 1;
}

const char *crazypod_now_playing_theme_choice_title(int index)
{
    return index == 0 ? CP_TR("Default") : "";
}

bool crazypod_now_playing_theme_choice_current(int index)
{
    return index == 0;
}

bool crazypod_now_playing_theme_select(int index)
{
    /* No stale theme configuration file is ever mutated or deleted. */
    if(index != 0)
        return false;
    crazypod_now_playing_theme_prepare();
    return true;
}

bool crazypod_now_playing_theme_enabled(void)
{
    return false;
}
bool crazypod_now_playing_theme_open(void)
{
    return false;
}
bool crazypod_now_playing_theme_modal_visible(void)
{
    return false;
}
bool crazypod_now_playing_theme_owns_status_bar(void)
{
    return false;
}
int crazypod_now_playing_theme_last_error(void)
{
    return 0;
}
bool crazypod_now_playing_theme_render(
    lv_obj_t *parent, uint32_t accent)
{
    (void)parent;
    (void)accent;
    return false; /* Always render the native Now Playing screen. */
}
bool crazypod_now_playing_theme_handle_input(
    const struct crazypod_input_event *event)
{
    (void)event;
    return false;
}
void crazypod_now_playing_theme_close(void)
{
}

#endif
