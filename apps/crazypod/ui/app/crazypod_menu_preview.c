#include "config.h"

#include "../../crazypod_l10n.h"

#ifdef IPOD_6G

#include "settings.h"

#include "../../crazypod_appearance.h"
#include "../../crazypod_artwork.h"
#include "../../crazypod_music.h"
#include "../../crazypod_playlist.h"
#include "../features/music/crazypod_music_feature.h"
#include "../features/settings/crazypod_settings_feature.h"
#include "../navigation/crazypod_route_registry.h"
#include "../presentation/crazypod_glass_slots.h"
#include "../presentation/crazypod_marquee.h"
#include "../presentation/crazypod_preview_motion.h"
#include "../presentation/crazypod_preview_primitives.h"
#include "../presentation/crazypod_ui_widgets.h"
#include "crazypod_menu_preview.h"

#define ARTWORK_CACHE_SIZE 72
#define MUSIC_ROOT_ARTWORK_SIZE 96
#define ARTWORK_PRIORITY 20
#define COLOR_WHITE 0xFFFFFF

static struct {
    struct crazypod_menu_preview_host host;
    bool motion_ready;
    bool defer_media;
} preview;

static uint32_t primary_color(void)
{
    return crazypod_appearance_color(
        crazypod_appearance_get()->primary_color);
}

static uint32_t secondary_color(void)
{
    return crazypod_appearance_color(
        crazypod_appearance_get()->secondary_color);
}

static lv_obj_t *preview_parent(void)
{
    return crazypod_preview_motion_parent(preview.host.parent);
}

bool crazypod_menu_preview_is_music_route(
    enum crazypod_route route)
{
    return crazypod_route_registry_has_flag(
        route, CRAZYPOD_ROUTE_FLAG_PREVIEW);
}

bool crazypod_menu_preview_is_skeuomorphic_route(
    enum crazypod_route route)
{
    return crazypod_route_registry_has_flag(
        route,
        CRAZYPOD_ROUTE_FLAG_SKEUOMORPHIC_PREVIEW);
}

static bool is_settings_route(enum crazypod_route route)
{
    return route >= SETTINGS_ROUTE_MENU &&
        route <= SETTINGS_ROUTE_MAIN_MENU_ACTIONS;
}

static void render_editor(
    const char *value, const char *empty_text,
    const char *detail)
{
    lv_obj_t *parent = preview_parent();
    lv_obj_t *card;
    lv_obj_t *symbol;
    lv_obj_t *label;

    card = crazypod_ui_widget_box(
        parent, crazypod_preview_centered_x(118),
        crazypod_preview_visual_y(64),
        118, 64, 14,
        primary_color(), 210);
    lv_obj_set_style_bg_grad_color(
        card, lv_color_hex(secondary_color()), 0);
    lv_obj_set_style_bg_grad_dir(
        card, LV_GRAD_DIR_HOR, 0);
    symbol = crazypod_ui_widget_label(
        card, LV_SYMBOL_KEYBOARD,
        &lv_font_montserrat_16, COLOR_WHITE, 225);
    lv_obj_set_pos(symbol, 10, 9);
    label = crazypod_ui_widget_label(
        card,
        value != NULL && value[0] != '\0'
            ? value : empty_text,
        &lv_font_montserrat_12, COLOR_WHITE,
        value != NULL && value[0] != '\0' ? 255 : 130);
    lv_obj_set_pos(label, 10, 35);
    lv_obj_set_size(label, 98, 23);
    crazypod_marquee_configure_centered(label, true);

    crazypod_preview_make_caption(
        parent, detail, &lv_font_montserrat_8,
        "", &lv_font_montserrat_8);
}

void crazypod_menu_preview_configure(
    const struct crazypod_menu_preview_host *host)
{
    if(host != NULL)
        preview.host = *host;
}

void crazypod_menu_preview_reset(void)
{
    preview.motion_ready = false;
    preview.defer_media = false;
}

void crazypod_menu_preview_render(
    const struct route_state *state, bool animated)
{
    struct crazypod_track track;
    bool have_track;
    bool animate = animated && preview.motion_ready &&
        crazypod_menu_preview_is_skeuomorphic_route(
            state->route);

    crazypod_preview_motion_set_profile(
        state->route == MUSIC_ROUTE_MENU
            ? CRAZYPOD_PREVIEW_PROFILE_MUSIC
            : CRAZYPOD_PREVIEW_PROFILE_DEFAULT);

    preview.defer_media = animate;
    *crazypod_preview_motion_media_deferred_flag() = false;
    crazypod_preview_motion_reset_root(preview.host.parent);
    if(state->route == MUSIC_ROUTE_MENU ||
       state->route == SETTINGS_ROUTE_MENU) {
        /* Full-width native root lists do not need album cards
         * or expensive right-hand preview animation on wheel ticks. */
        preview.defer_media = false;
        preview.motion_ready = true;
        return;
    }
    if(state->route == MUSIC_ROUTE_SEARCH) {
        render_editor(
            crazypod_music_search_query(), CP_TR("Any track"),
            CP_TR("Searches title, artist and album."));
    }
    else if(is_settings_route(state->route)) {
        crazypod_settings_feature_render_preview(
            preview_parent(), state,
            preview.host.item_title(
                state, state->selected),
            primary_color(), secondary_color(),
            global_settings.eq_enabled,
            global_settings.playlist_shuffle,
            crazypod_queue_repeat() != REPEAT_OFF);
    }
    else {
        have_track = crazypod_music_feature_copy_route_track(
            state, state->selected, &track);
        crazypod_music_feature_render_item_preview(
            preview_parent(), state, have_track ? &track : NULL,
            preview.host.metadata_font);
    }
    preview.defer_media = false;
    preview.motion_ready = true;
    if(animate)
        crazypod_preview_motion_start_entrance();
}

void crazypod_menu_preview_prefetch(
    const struct route_state *state)
{
    struct crazypod_track track;
    bool have_track = false;
    if(state == NULL)
        return;
    if(state->route == MUSIC_ROUTE_MENU ||
       state->route == SETTINGS_ROUTE_MENU) {
        /* No preview pane exists in these native full-width lists. */
        return;
    }
    if(state->route == MUSIC_ROUTE_ALBUMS)
        have_track = crazypod_music_copy_album_track(
            state->selected, 0, &track);
    else if(state->route == MUSIC_ROUTE_ARTISTS ||
            state->route == MUSIC_ROUTE_PLAYLISTS)
        return;
    else if(crazypod_menu_preview_is_music_route(
                state->route))
        have_track = crazypod_music_feature_copy_route_track(
            state, state->selected, &track);
    if(have_track)
        (void)crazypod_artwork_load_priority(
            CRAZYPOD_PREVIEW_ARTWORK_SLOT, &track,
            ARTWORK_CACHE_SIZE, ARTWORK_PRIORITY);
}

void crazypod_menu_preview_settle(void)
{
    crazypod_preview_motion_settle();
}

bool crazypod_menu_preview_motion_ready(void)
{
    return preview.motion_ready;
}

#endif
