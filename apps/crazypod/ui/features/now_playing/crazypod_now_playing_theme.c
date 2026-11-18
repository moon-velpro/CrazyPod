#include "config.h"

#include "../../../crazypod_l10n.h"

#ifdef IPOD_6G

#include "../../../crazypod_artwork.h"
#include "crazypod_now_playing_feature.h"

/* CrazyPod is now a dedicated music player. External CPK/miniapp
 * Now Playing themes are retired; the native playback screen remains.
 * Keep these entry points temporarily so existing callers can be
 * untangled without changing the playback or album-art interfaces. */
static bool theme_prepared;

void crazypod_now_playing_theme_prepare(void)
{
    if(theme_prepared)
        return;
    crazypod_now_playing_artwork_set_source_size(
        CRAZYPOD_ARTWORK_CACHE_SIZE);
    theme_prepared = true;
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
    return false;
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
