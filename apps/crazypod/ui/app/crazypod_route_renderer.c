#include "config.h"

#ifdef IPOD_6G

#include "lcd.h"

#include "../../crazypod_appearance.h"
#include "../../crazypod_coverflow.h"
#include "../../crazypod_wallpaper.h"
#include "../features/music/crazypod_music_feature.h"
#include "../features/now_playing/crazypod_now_playing_feature.h"
#include "../features/settings/crazypod_settings_feature.h"
#include "../navigation/crazypod_route_registry.h"
#include "../presentation/crazypod_alpha_jump_hud.h"
#include "../presentation/crazypod_glass_slots.h"
#include "../presentation/crazypod_menu_list.h"
#include "../presentation/crazypod_menu_screen.h"
#include "../presentation/crazypod_preview_motion.h"
#include "../presentation/crazypod_screen_corners.h"
#include "../presentation/crazypod_ui_widgets.h"
#include "../shell/crazypod_shell.h"
#include "../shell/crazypod_home_actions.h"
#include "../shell/crazypod_status_bar.h"
#include "crazypod_choice_coordinator.h"
#include "crazypod_menu_preview.h"
#include "crazypod_route_renderer.h"

#define STATUS_BAR_HEIGHT 32
#define MENU_PANEL_Y STATUS_BAR_HEIGHT
#define MENU_PANEL_HEIGHT (LCD_HEIGHT - MENU_PANEL_Y)
#define MENU_PANEL_WIDTH 160
#define COLOR_DETAIL 0x08080D
#define COLOR_PANEL 0x1B1B22
#define COLOR_WHITE 0xFFFFFF

static struct crazypod_route_renderer_host host;

static void reset_feature_surfaces(void)
{
    if(crazypod_coverflow_active())
        crazypod_coverflow_leave();
    crazypod_alpha_jump_hud_reset();
    crazypod_now_playing_overlay_reset();
    crazypod_choice_coordinator_reset();
    crazypod_menu_list_clear();
    crazypod_preview_motion_forget();
    crazypod_menu_preview_reset();
    crazypod_now_playing_feature_reset_screen();
    crazypod_music_feature_reset_view();
}

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

static void create_panel_backgrounds(void)
{
    lv_obj_t *top;
    lv_obj_t *left;
    bool prepared;

    prepared = crazypod_glass_slot_prepare_menu(
        CRAZYPOD_GLASS_SLOT_MENU_TOPBAR,
        0, 0, LCD_WIDTH, STATUS_BAR_HEIGHT,
        CRAZYPOD_GLASS_MENU_TOPBAR);
    top = crazypod_glass_slot_panel(
        CRAZYPOD_GLASS_SLOT_MENU_TOPBAR, prepared,
        crazypod_shell_product_content(), 0, 0,
        LCD_WIDTH, STATUS_BAR_HEIGHT, 0,
        CRAZYPOD_GLASS_MENU_TOPBAR);
    prepared = crazypod_glass_slot_prepare_menu(
        CRAZYPOD_GLASS_SLOT_MENU_PANEL,
        0, MENU_PANEL_Y,
        MENU_PANEL_WIDTH, MENU_PANEL_HEIGHT,
        CRAZYPOD_GLASS_MENU_PANEL);
    left = crazypod_glass_slot_panel(
        CRAZYPOD_GLASS_SLOT_MENU_PANEL, prepared,
        crazypod_shell_product_content(), 0, MENU_PANEL_Y,
        MENU_PANEL_WIDTH, MENU_PANEL_HEIGHT, 0,
        CRAZYPOD_GLASS_MENU_PANEL);
    lv_obj_remove_flag(top, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_border_width(top, 1, 0);
    lv_obj_set_style_border_side(
        top, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(
        top, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_set_style_border_opa(top, 22, 0);
    lv_obj_set_style_border_width(left, 1, 0);
    lv_obj_set_style_border_side(
        left, LV_BORDER_SIDE_RIGHT, 0);
    lv_obj_set_style_border_color(
        left, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_set_style_border_opa(left, 22, 0);
}

static lv_obj_t *make_search_panel(
    lv_obj_t *parent, enum crazypod_glass_slot slot,
    int x, int y, int width, int height, int radius)
{
    bool prepared = crazypod_glass_slot_prepare_frame(
        slot, x, y, width, height,
        CRAZYPOD_GLASS_TEXT_PANEL);

    return crazypod_glass_slot_panel(
        slot, prepared, parent, x, y,
        width, height, radius,
        CRAZYPOD_GLASS_TEXT_PANEL);
}

static void render_menu(const struct route_state *state)
{
    int item_count = host.item_count(state);
    const struct crazypod_menu_screen_context context = {
        .parent = crazypod_shell_product_content(),
        .item_count = item_count,
        .primary_color = primary_color(),
        .secondary_color = secondary_color(),
        .panel_color = COLOR_PANEL,
        .gradient_highlight =
            crazypod_appearance_get()->highlight_style != 0,
        .metadata_font = host.metadata_font,
        .item_title = host.item_title,
        .item_is_current = host.item_is_current,
    };

    /*
     * A hierarchical menu owns the entire content area. Preview objects
     * must not overlap list rows or turn this back into a split Android UI.
     * The status bar remains owned by Shell above STATUS_BAR_HEIGHT.
     */
    {
        lv_obj_t *page = crazypod_ui_widget_box(
            crazypod_shell_product_content(),
            0, STATUS_BAR_HEIGHT,
            LCD_WIDTH, LCD_HEIGHT - STATUS_BAR_HEIGHT,
            0, 0xF6F9FC, LV_OPA_COVER);
        lv_obj_remove_flag(page, LV_OBJ_FLAG_CLICKABLE);
    }
    crazypod_menu_screen_render(state, &context);
}

static void render_music(const struct route_state *state)
{
    if(state->route == MUSIC_ROUTE_SEARCH)
        create_panel_backgrounds();
    if(crazypod_music_feature_render_special(
           crazypod_shell_product_content(), state,
           host.metadata_font, host.item_count(state),
           host.item_title, primary_color(), secondary_color(),
           COLOR_PANEL,
           crazypod_appearance_get()->highlight_style != 0,
           make_search_panel)) {
        return;
    }
    render_menu(state);
}

static void render_now_playing(
    const struct route_state *state)
{
    if(state->route == MUSIC_ROUTE_NOW_PLAYING) {
        const struct crazypod_now_playing_render_context context = {
            .parent = crazypod_shell_product_content(),
            .metadata_font = host.metadata_font,
            .render_artwork = host.render_artwork,
            .boost = host.boost,
        };

        crazypod_now_playing_feature_render(&context);
        return;
    }
    render_menu(state);
}

static void render_feature(const struct route_state *state)
{
    const struct crazypod_feature *feature =
        crazypod_route_registry_feature(state->route);

    if(feature == NULL) {
        if(crazypod_home_actions_is_adjustment(state->route))
            crazypod_home_actions_render_adjustment(
                state->route, crazypod_shell_product_content());
        else
            render_menu(state);
        return;
    }
    switch(feature->id) {
    case CRAZYPOD_FEATURE_MUSIC:
        render_music(state);
        return;
    case CRAZYPOD_FEATURE_NOW_PLAYING:
        render_now_playing(state);
        return;
    case CRAZYPOD_FEATURE_SETTINGS:
        if(crazypod_settings_feature_render(
               state, crazypod_shell_product_content(),
               host.metadata_font, primary_color()))
            return;
        break;
    default:
        break;
    }
    render_menu(state);
}

void crazypod_route_renderer_configure(
    const struct crazypod_route_renderer_host *new_host)
{
    if(new_host != NULL)
        host = *new_host;
}

void crazypod_route_renderer_render(
    const struct route_state *state, long now,
    bool transition)
{
    const lv_image_dsc_t *menu_wallpaper;
    lv_obj_t *content;
    bool solid_black;
    bool home_actions;
    uint32_t background;
    uint32_t foreground;

    (void)now;
    if(state == NULL ||
       crazypod_route_registry_get(state->route) == NULL)
        return;

    solid_black = crazypod_route_registry_has_flag(
        state->route, CRAZYPOD_ROUTE_FLAG_SOLID_BLACK);
    home_actions = crazypod_home_actions_is_route(state->route);
    background = home_actions ? 0xF6F9FC
        : solid_black ? 0x000000
        : crazypod_appearance_menu_color();
    foreground = home_actions ? 0x1E2938 : COLOR_WHITE;
    content = crazypod_shell_product_content();

    reset_feature_surfaces();
    lv_obj_clean(content);
    lv_obj_set_pos(content, 0, 0);
    lv_obj_set_style_bg_color(
        content, lv_color_hex(background), 0);
    lv_obj_set_style_bg_opa(content, LV_OPA_COVER, 0);
    crazypod_ui_widget_box(
        content, 0, 0, LCD_WIDTH, LCD_HEIGHT, 0,
        background, LV_OPA_COVER);
    menu_wallpaper = solid_black
        ? NULL : crazypod_custom_menu_wallpaper();
    if(menu_wallpaper != NULL) {
        lv_obj_t *image = lv_image_create(content);

        lv_image_set_src(image, menu_wallpaper);
        lv_obj_set_pos(image, 0, 0);
        lv_obj_remove_flag(image, LV_OBJ_FLAG_CLICKABLE);
    }
    render_feature(state);
    lv_obj_invalidate(content);
    if(transition) {
        lv_obj_set_x(content, 0);
        lv_obj_set_style_opa(content, LV_OPA_COVER, 0);
        lv_obj_invalidate(content);
    }
    crazypod_status_bar_set_visible(1, true);
    crazypod_status_bar_set_palette(
        1, foreground, background);
    crazypod_status_bar_foreground(1);
    crazypod_screen_corners_refresh();
}

void crazypod_route_renderer_prepare_loading(void)
{
    reset_feature_surfaces();
    lv_obj_clean(crazypod_shell_product_content());
    crazypod_status_bar_set_palette(
        1, COLOR_WHITE, COLOR_DETAIL);
    lv_obj_set_style_bg_color(
        crazypod_shell_product_content(),
        lv_color_hex(COLOR_DETAIL), 0);
    lv_obj_set_style_bg_opa(
        crazypod_shell_product_content(), LV_OPA_COVER, 0);
}

#endif
