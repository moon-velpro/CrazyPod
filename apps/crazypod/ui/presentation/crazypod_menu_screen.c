#include "config.h"

#include "../../crazypod_l10n.h"

#ifdef IPOD_6G

#include "lvgl.h"

#include "crazypod_ui_menu_layout.h"
#include "crazypod_ui_widgets.h"
#include "crazypod_empty_state.h"
#include "crazypod_menu_icon_assets.h"
#include "crazypod_menu_list.h"
#include "../navigation/crazypod_route_query.h"
#include "crazypod_marquee.h"
#include "crazypod_menu_screen.h"

#define COLOR_WHITE 0xFFFFFF
#define COLOR_TEXT 0x1E2938
#define COLOR_ACCENT 0x447AA9
#define COLOR_MUTED 0x758293
#define COLOR_SELECTED 0xDDEAF7
#define COLOR_LINE 0xE7ECF2
#define CRAZYPOD_VISIBLE_ROWS 6
#define CRAZYPOD_MENU_HEADER_X 16
#define CRAZYPOD_MENU_HEADER_Y 38
#define CRAZYPOD_MENU_HEADER_WIDTH 288
#define CRAZYPOD_MENU_HEADER_HEIGHT 24
#define CRAZYPOD_MENU_ROW_X 8
#define CRAZYPOD_MENU_ROW_Y 64
#define CRAZYPOD_MENU_ROW_WIDTH 304
#define CRAZYPOD_MENU_ROW_HEIGHT 28
#define CRAZYPOD_MENU_ROW_STEP 28
#define CRAZYPOD_MENU_SCROLL_X 314
#define CRAZYPOD_MENU_SCROLL_Y 68
#define CRAZYPOD_MENU_SCROLL_HEIGHT 160

static lv_obj_t *make_box(
    lv_obj_t *parent, int x, int y, int width, int height,
    int radius, uint32_t color, lv_opa_t opacity)
{
    return crazypod_ui_widget_box(
        parent, x, y, width, height, radius, color, opacity);
}

static lv_obj_t *make_label(
    lv_obj_t *parent, const char *text, const lv_font_t *font,
    uint32_t color, lv_opa_t opacity)
{
    return crazypod_ui_widget_label(
        parent, text, font, color, opacity);
}

void crazypod_menu_screen_render(
    const struct route_state *state,
    const struct crazypod_menu_screen_context *context)
{
    int count = context->item_count;
    int start;
    int row;
    lv_obj_t *header;

    crazypod_menu_list_reset(state->route);
    header = make_label(context->parent, crazypod_route_query_title(state),
                        context->metadata_font,
                        COLOR_TEXT, LV_OPA_COVER);
    lv_obj_set_pos(header, CRAZYPOD_MENU_HEADER_X,
                   CRAZYPOD_MENU_HEADER_Y);
    lv_obj_set_width(header, CRAZYPOD_MENU_HEADER_WIDTH);
    lv_obj_set_height(header, CRAZYPOD_MENU_HEADER_HEIGHT);
    lv_label_set_long_mode(header, LV_LABEL_LONG_MODE_DOTS);

    if(count <= 0) {
        if(state->route == DIY_ROUTE_WALLPAPER_FILES)
            crazypod_empty_state_render(
                context->parent, LV_SYMBOL_IMAGE, CP_TR("No Pictures"),
                CP_TR("Add JPG or BMP files to /Pictures."));
        else if(state->route == PHOTOS_ROUTE_LIBRARY)
            crazypod_empty_state_render(context->parent,
                LV_SYMBOL_IMAGE,
                CP_TR("No Pictures"),
                CP_TR("Add JPG, JPEG or BMP files to /Pictures."));
        else if(state->route == PHOTOS_ROUTE_VIDEOS ||
                state->route == PHOTOS_ROUTE_DELETE_VIDEOS)
            crazypod_empty_state_render(context->parent,
                LV_SYMBOL_PLAY,
                CP_TR("No Videos"),
                CP_TR("Convert MPG or MPEG files into /Videos."));
        else if(state->route == PHOTOS_ROUTE_FAVORITES)
            crazypod_empty_state_render(context->parent,
                LV_SYMBOL_IMAGE,
                CP_TR("No Favorites"),
                               CP_TR("Hold Select on a photo to save it here."));
        else if(state->route == EXTRAS_ROUTE_MENU)
            crazypod_empty_state_render(context->parent,
                LV_SYMBOL_DIRECTORY,
                CP_TR("Nothing Hidden"),
                CP_TR("Hide apps in Settings > Main Menu."));
        else if(state->route == UTILITIES_ROUTE_MENU)
            crazypod_empty_state_render(context->parent,
                LV_SYMBOL_FILE,
                CP_TR("No Mini Apps"),
                CP_TR("Copy a CPK directly to /MiniApps."));
        else if(state->route == NOTES_ROUTE_DELETED)
            crazypod_empty_state_render(context->parent,
                LV_SYMBOL_EDIT,
                CP_TR("Deleted Is Empty"),
                CP_TR("Deleted notes can be restored from here."));
        else if(state->route == BOOKS_ROUTE_LIBRARY)
            crazypod_empty_state_render(context->parent,
                LV_SYMBOL_FILE,
                CP_TR("No Books"),
                CP_TR("Add EPUB, TXT or Markdown files to /Books."));
        else if(state->route == BOOKS_ROUTE_RECENTS)
            crazypod_empty_state_render(context->parent,
                LV_SYMBOL_FILE,
                CP_TR("No Recent Books"),
                CP_TR("Open a book to add it here."));
        else if(state->route == BOOKS_ROUTE_FAVORITES)
            crazypod_empty_state_render(context->parent,
                LV_SYMBOL_FILE,
                CP_TR("No Favorites"),
                CP_TR("Favorite a book from Book Actions."));
        else if(state->route == BOOKS_ROUTE_BOOKMARKS)
            crazypod_empty_state_render(context->parent,
                LV_SYMBOL_FILE,
                CP_TR("No Bookmark"),
                CP_TR("Press PLAY while reading to save this page."));
        else if(state->route == PODCASTS_ROUTE_MENU)
            crazypod_empty_state_render(context->parent,
                LV_SYMBOL_AUDIO,
                CP_TR("No Podcasts"),
                CP_TR("Add audio files under /Podcasts and rescan."));
        else if(state->route == CONTACTS_ROUTE_LIST)
            crazypod_empty_state_render(context->parent,
                NULL,
                CP_TR("No Contacts"),
                CP_TR("Add VCF files to /Contacts."));
        else if(state->route == WORKOUT_ROUTE_HISTORY)
            crazypod_empty_state_render(context->parent,
                LV_SYMBOL_PLAY,
                CP_TR("No Workouts"),
                CP_TR("Start a time-only workout first."));
        else
            crazypod_empty_state_render(
                context->parent, LV_SYMBOL_AUDIO, CP_TR("Nothing Here"),
                CP_TR("Add local music and rescan."));
        return;
    }

    start = crazypod_ui_menu_window_start(
        count, state->selected, CRAZYPOD_VISIBLE_ROWS);

    for(row = 0; row < CRAZYPOD_VISIBLE_ROWS; ++row) {
        int index = start + row;
        int y = CRAZYPOD_MENU_ROW_Y + row * CRAZYPOD_MENU_ROW_STEP;
        bool selected = index == state->selected;
        lv_obj_t *row_box;
        lv_obj_t *label;
        lv_obj_t *marker;
        const char *title;
        int text_x = 14;
        int text_width = 250;

        if(index >= count)
            break;
        /* Full-width, restrained hierarchy: selection is not an app tile. */
        row_box = make_box(context->parent,
                           CRAZYPOD_MENU_ROW_X, y,
                           CRAZYPOD_MENU_ROW_WIDTH,
                           CRAZYPOD_MENU_ROW_HEIGHT, 7,
                           selected ? COLOR_SELECTED : COLOR_WHITE,
                           LV_OPA_COVER);
        lv_obj_set_style_border_width(row_box, 0, 0);
        if(selected)
            make_box(row_box, 1, 5, 3, 18, 2,
                     COLOR_ACCENT, LV_OPA_COVER);
        else
            make_box(row_box, 13, CRAZYPOD_MENU_ROW_HEIGHT - 1,
                     CRAZYPOD_MENU_ROW_WIDTH - 26, 1, 0,
                     COLOR_LINE, LV_OPA_COVER);
        title = context->item_title(state, index);
        if(title == NULL)
            title = "";
        label = make_label(row_box, title, context->metadata_font,
                           COLOR_TEXT, selected ? 255 : 220);
        lv_obj_set_width(label, text_width);
        crazypod_ui_widget_align_row_label(
            label, text_x, CRAZYPOD_UI_ROW_LABEL_TEXT);
        crazypod_marquee_configure(label, selected);

        marker = make_label(row_box,
                            context->item_is_current(state, index)
                                ? LV_SYMBOL_OK :
                            LV_SYMBOL_RIGHT,
                            &lv_font_montserrat_8,
                            COLOR_ACCENT, selected ? 255 : 135);
        crazypod_ui_widget_align_row_label(
            marker, 280, CRAZYPOD_UI_ROW_LABEL_MARKER);
        crazypod_menu_list_bind_row(row, row_box, label, marker);
    }

    if(count > CRAZYPOD_VISIBLE_ROWS) {
        int track_height = CRAZYPOD_MENU_SCROLL_HEIGHT;
        int thumb_height;
        int thumb_y;
        lv_obj_t *bar;
        crazypod_ui_menu_scroll_thumb(
            count, state->selected, CRAZYPOD_VISIBLE_ROWS,
            CRAZYPOD_MENU_SCROLL_Y, track_height, 12,
            &thumb_y, &thumb_height);
        bar = make_box(context->parent, CRAZYPOD_MENU_SCROLL_X,
                       CRAZYPOD_MENU_SCROLL_Y, 2, track_height, 1,
                       COLOR_MUTED, 35);
        (void)bar;
        bar = make_box(context->parent, CRAZYPOD_MENU_SCROLL_X,
                       thumb_y, 2, thumb_height, 1, COLOR_ACCENT, 180);
        crazypod_menu_list_bind_scroll_thumb(bar);
    }

}



#endif
