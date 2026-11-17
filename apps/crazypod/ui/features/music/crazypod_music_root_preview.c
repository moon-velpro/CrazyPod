#include "config.h"

#include "../../../crazypod_l10n.h"

#ifdef IPOD_6G

#include <stdio.h>

#include "../../../crazypod_artwork.h"
#include "../../../crazypod_music.h"
#include "../../../crazypod_playlist.h"
#include "../../presentation/crazypod_liquid_music.h"
#include "../../presentation/crazypod_preview_motion.h"
#include "../../presentation/crazypod_preview_primitives.h"
#include "../../presentation/crazypod_ui_widgets.h"
#include "crazypod_music_root_preview.h"

#define CRAZYPOD_MENU_ARTWORK_PRIORITY 20

static const char *const music_menu_titles[] = {
    CP_TR("Now Playing"), CP_TR("Album Flow"),
    CP_TR("All Music"), CP_TR("Playlists"),
    CP_TR("Artists"), CP_TR("Albums"),
    CP_TR("Songs"), CP_TR("Search"),
};

static bool copy_current_track(struct crazypod_track *track)
{
    char path[MAX_PATH];
    int index = crazypod_queue_copy_path(
            crazypod_queue_index(), path, sizeof(path))
        ? crazypod_music_find_track(path) : -1;
    return crazypod_music_copy_track(index, track);
}

/* Liquid Music preview uses actual cached cover art for music-facing
 * destinations and quiet symbols for library actions. No fake hardware or
 * continuously animated decorations are needed on a 320x240 iPod. */
static void render_liquid_music_root_preview(
    lv_obj_t *parent, int selected, bool defer_media_value)
{
    struct crazypod_track current;
    struct crazypod_track album_track;
    const struct crazypod_track *track = NULL;
    const lv_image_dsc_t *artwork = NULL;
    lv_obj_t *card;
    lv_obj_t *symbol;
    char count_text[96];
    int count;

    if(selected == 0 && copy_current_track(&current))
        track = &current;
    else if((selected == 1 || selected == 5) &&
            crazypod_music_album_count() > 0 &&
            crazypod_music_copy_album_track(0, 0, &album_track))
        track = &album_track;

    if(selected == 1 || selected == 5)
        count = crazypod_music_album_count();
    else if(selected == 3)
        count = crazypod_music_playlist_count();
    else if(selected == 4)
        count = crazypod_music_artist_count();
    else
        count = crazypod_music_track_count();

    if(selected == 0 && track != NULL)
        snprintf(count_text, sizeof(count_text), "%s", track->artist);
    else if(selected == 7)
        snprintf(count_text, sizeof(count_text),
                 CP_FMT("%d local tracks"), count);
    else
        snprintf(count_text, sizeof(count_text),
                 selected == 3 ? CP_FMT("%d playlists") :
                 selected == 4 ? CP_FMT("%d artists") :
                 selected == 1 || selected == 5
                     ? CP_FMT("%d albums") : CP_FMT("%d songs"),
                 count);

    card = crazypod_ui_widget_box(
        parent, 187, 48, 114, 114, 14,
        CRAZYPOD_LM_GLASS, CRAZYPOD_LM_GLASS_FILL_OPA);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_color(
        card, lv_color_hex(CRAZYPOD_LM_TEXT), 0);
    lv_obj_set_style_border_opa(
        card, CRAZYPOD_LM_GLASS_BORDER_OPA, 0);
    lv_obj_remove_flag(card, LV_OBJ_FLAG_CLICKABLE);

    if(track != NULL) {
        artwork = defer_media_value
            ? crazypod_artwork_load_cached_priority(
                  CRAZYPOD_PREVIEW_ARTWORK_SLOT, track, 96, 180)
            : crazypod_artwork_load_priority(
                  CRAZYPOD_PREVIEW_ARTWORK_SLOT, track, 96,
                  CRAZYPOD_MENU_ARTWORK_PRIORITY);
        if(artwork == NULL && defer_media_value)
            *crazypod_preview_motion_media_deferred_flag() = true;
    }

    if(artwork != NULL) {
        lv_obj_t *image = lv_image_create(card);

        lv_image_set_src(image, artwork);
        lv_obj_center(image);
        lv_obj_remove_flag(image, LV_OBJ_FLAG_CLICKABLE);
    }
    else {
        const char *glyph = selected == 7 ? LV_SYMBOL_SEARCH :
            selected == 4 ? LV_SYMBOL_DIRECTORY :
            selected == 1 || selected == 5 ? LV_SYMBOL_IMAGE :
            selected == 2 || selected == 6 ? LV_SYMBOL_LIST :
            LV_SYMBOL_AUDIO;

        symbol = crazypod_ui_widget_label(
            card, glyph, &lv_font_montserrat_24,
            CRAZYPOD_LM_TEXT, 188);
        lv_obj_center(symbol);
    }

    (void)crazypod_preview_make_caption(
        parent,
        selected == 0 && track != NULL
            ? track->title : music_menu_titles[selected],
        &lv_font_montserrat_12,
        count_text, &lv_font_montserrat_8);
}

void crazypod_music_root_preview_render(
    lv_obj_t *parent, int selected, bool defer_media)
{
    if(parent == NULL || selected < 0 || selected >= 8)
        return;
    render_liquid_music_root_preview(parent, selected, defer_media);
}

#endif
