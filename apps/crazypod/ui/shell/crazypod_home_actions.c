#include "config.h"

#include "../../crazypod_l10n.h"

#ifdef IPOD_6G

#include <stdio.h>
#include <string.h>

#include "backlight.h"
#include "button.h"
#include "iap-usb.h"
#include "misc.h"
#include "settings.h"
#include "sound.h"

#include "../../crazypod_state.h"
#include "../presentation/crazypod_menu_icon_assets.h"
#include "../presentation/crazypod_overlay_glass.h"
#include "../presentation/crazypod_popup_layout.h"
#include "../presentation/crazypod_popup_motion.h"
#include "../presentation/crazypod_ui_widgets.h"
#include "crazypod_desktop_native.h"
#include "crazypod_home_actions.h"

#define COLOR_WHITE 0xFFFFFF
#define COLOR_ACCENT 0x477DB7
#define COLOR_INK 0x213248
#define COLOR_MUTED 0x586E84
#define COLOR_SKY 0xE7F2FF
#define HOME_ACTION_COUNT 3
#define HOME_ACTION_ICON_SIZE 28
#define HOME_ACTION_ICON_CIRCLE_SIZE 42

enum home_action {
    HOME_ACTION_QUEUE = 0,
    HOME_ACTION_BRIGHTNESS,
    HOME_ACTION_VOLUME,
};

struct home_actions_state {
    lv_obj_t *parent;
    lv_obj_t *root;
    lv_obj_t *panel;
    lv_obj_t *cells[HOME_ACTION_COUNT];
    lv_obj_t *icons[HOME_ACTION_COUNT];
    lv_obj_t *names[HOME_ACTION_COUNT];
    lv_obj_t *detail;
    lv_obj_t *level_bar;
    lv_obj_t *level_value;
    enum home_action selected;
    bool adjusting;
    struct crazypod_home_actions_callbacks callbacks;
};

static struct home_actions_state actions;

static const char *action_title(enum home_action action)
{
    switch(action) {
    case HOME_ACTION_QUEUE:
        return CP_TR("Playback Queue");
    case HOME_ACTION_BRIGHTNESS:
        return CP_TR("Brightness");
    case HOME_ACTION_VOLUME:
        return CP_TR("VOLUME");
    }
    return "";
}

static enum crazypod_menu_icon action_icon(enum home_action action)
{
    switch(action) {
    case HOME_ACTION_QUEUE:
        return CRAZYPOD_MENU_ICON_QUEUE;
    case HOME_ACTION_BRIGHTNESS:
        return CRAZYPOD_MENU_ICON_BRIGHTNESS;
    case HOME_ACTION_VOLUME:
        return CRAZYPOD_MENU_ICON_SPEAKER;
    }
    return CRAZYPOD_MENU_ICON_NONE;
}

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

static void clear_objects(void)
{
    actions.root = NULL;
    actions.panel = NULL;
    memset(actions.cells, 0, sizeof(actions.cells));
    memset(actions.icons, 0, sizeof(actions.icons));
    memset(actions.names, 0, sizeof(actions.names));
    actions.detail = NULL;
    actions.level_bar = NULL;
    actions.level_value = NULL;
}

static void delete_overlay(void)
{
    if(actions.panel != NULL && lv_obj_is_valid(actions.panel))
        lv_anim_delete(actions.panel, NULL);
    if(actions.root != NULL && lv_obj_is_valid(actions.root))
        lv_obj_delete(actions.root);
    clear_objects();
}

static void begin_overlay(int width, int height)
{
    struct crazypod_popup_geometry geometry =
        crazypod_popup_centered_geometry(width, height);
    bool replacing = actions.root != NULL;

    delete_overlay();
    /* When switching the quick controls to the adjustment dial, discard
     * the previous modal before capturing its frosted Home backdrop.
     * This redraw happens once per modal change, never during wheel scroll. */
    if(replacing)
        lv_refr_now(NULL);
    actions.root = make_box(
        actions.parent, 0, 0, LCD_WIDTH, LCD_HEIGHT,
        0, 0x365675, 65);
    lv_obj_remove_flag(actions.root, LV_OBJ_FLAG_CLICKABLE);
    (void)crazypod_desktop_native_create_modal_underlay(actions.root);
    /* GlassVGL-style cached blur + subtle Liquid Glass edge glints:
     * render the RGB565 material once; keep audio and the wheel hot path
     * free of live blur, displacement maps and shadow animations. */
    actions.panel = crazypod_overlay_glass_home_controls_panel(
        actions.root, geometry.x, geometry.y,
        geometry.width, geometry.height);
    crazypod_popup_animate(actions.panel, geometry.y);
}

static int action_level_percent(enum home_action action)
{
    int value;
    int minimum;
    int maximum;

    if(action == HOME_ACTION_BRIGHTNESS) {
        value = global_settings.brightness;
        minimum = MIN_BRIGHTNESS_SETTING;
        maximum = MAX_BRIGHTNESS_SETTING;
    }
    else if(action == HOME_ACTION_VOLUME) {
        value = global_status.volume;
        minimum = sound_min(SOUND_VOLUME);
        maximum = sound_max(SOUND_VOLUME);
    }
    else
        return 0;

    if(maximum <= minimum)
        return 0;
    return (value - minimum) * 100 / (maximum - minimum);
}

static void refresh_action_selection(void)
{
    int index;

    for(index = 0; index < HOME_ACTION_COUNT; ++index) {
        bool selected = index == (int)actions.selected;

        /* A selected iOS-style setting row replaces Android-like circles
         * and individual glass cards. No blur or re-layout on detents. */
        lv_obj_set_style_bg_color(
            actions.cells[index],
            lv_color_hex(selected ? 0xE2ECFC : COLOR_WHITE), 0);
        lv_obj_set_style_bg_opa(
            actions.cells[index], selected ? 235 : 160, 0);
        lv_obj_set_style_border_width(actions.cells[index], 1, 0);
        lv_obj_set_style_border_color(
            actions.cells[index], lv_color_hex(COLOR_WHITE), 0);
        lv_obj_set_style_border_opa(
            actions.cells[index], selected ? 220 : 120, 0);
        lv_obj_set_style_opa(
            actions.icons[index], selected ? 255 : 205, 0);
        if(actions.names[index] != NULL)
            lv_obj_set_style_text_color(
                actions.names[index], lv_color_hex(COLOR_INK), 0);
    }
}

static void show_action_list(void)
{
    const int width = 272;
    const int height = 196;
    const int inset = 12;
    const int row_height = 43;
    const int row_gap = 3;
    lv_obj_t *title;
    int index;

    actions.adjusting = false;
    begin_overlay(width, height);
    title = make_label(
        actions.panel, CP_TR("Controls"),
        &lv_font_montserrat_16, COLOR_INK, LV_OPA_COVER);
    lv_obj_set_pos(title, inset + 3, 10);
    lv_obj_set_width(title, width - 2 * inset);

    for(index = 0; index < HOME_ACTION_COUNT; ++index) {
        const enum home_action action = (enum home_action)index;
        const lv_image_dsc_t *asset =
            crazypod_menu_icon_asset(action_icon(action));
        lv_obj_t *icon_back;
        int y = 43 + index * (row_height + row_gap);

        actions.cells[index] = make_box(
            actions.panel, inset, y, width - 2 * inset,
            row_height, 13, COLOR_WHITE, 165);
        icon_back = make_box(
            actions.cells[index], 8, 5, 32, 32, 9,
            0xF0F4FA, LV_OPA_COVER);
        actions.icons[index] = lv_image_create(icon_back);
        lv_image_set_src(actions.icons[index], asset);
        lv_image_set_scale(
            actions.icons[index],
            20 * LV_SCALE_NONE / asset->header.w);
        lv_obj_set_style_image_recolor(
            actions.icons[index], lv_color_hex(COLOR_INK), 0);
        lv_obj_set_style_image_recolor_opa(
            actions.icons[index], LV_OPA_COVER, 0);
        lv_obj_center(actions.icons[index]);
        actions.names[index] = make_label(
            actions.cells[index], action_title(action),
            &lv_font_montserrat_12, COLOR_INK, LV_OPA_COVER);
        lv_obj_set_pos(actions.names[index], 48, 11);
        lv_obj_set_width(actions.names[index], 122);
        lv_label_set_long_mode(
            actions.names[index], LV_LABEL_LONG_MODE_DOTS);

        if(action == HOME_ACTION_QUEUE) {
            lv_obj_t *chevron = make_label(
                actions.cells[index], LV_SYMBOL_RIGHT,
                &lv_font_montserrat_10, COLOR_MUTED, 185);
            lv_obj_set_pos(chevron, 226, 11);
        }
        else {
            int fill_width;
            int percent = action_level_percent(action);

            if(percent < 0)
                percent = 0;
            if(percent > 100)
                percent = 100;
            make_box(actions.cells[index], 176, 20, 61, 4,
                     LV_RADIUS_CIRCLE, 0xCBD4E2, LV_OPA_COVER);
            fill_width = 61 * percent / 100;
            if(fill_width > 0)
                make_box(actions.cells[index], 176, 20,
                         fill_width, 4, LV_RADIUS_CIRCLE,
                         COLOR_ACCENT, LV_OPA_COVER);
        }
    }
    actions.detail = make_label(
        actions.panel, CP_TR("Wheel selects"),
        &lv_font_montserrat_8, COLOR_MUTED, 210);
    lv_obj_set_pos(actions.detail, 14, 181);
    lv_obj_set_width(actions.detail, width - 28);
    lv_obj_set_style_text_align(
        actions.detail, LV_TEXT_ALIGN_CENTER, 0);
    refresh_action_selection();
}

static int current_level_percent(void)
{
    return action_level_percent(actions.selected);
}

static void refresh_adjustment(void)
{
    char percent[8];
    int value = current_level_percent();

    if(value < 0)
        value = 0;
    if(value > 100)
        value = 100;
    snprintf(percent, sizeof(percent), "%d%%", value);
    lv_bar_set_value(actions.level_bar, value, LV_ANIM_OFF);
    CP_LV_LABEL_SET_TEXT(actions.level_value, percent);
}

static void show_adjustment(void)
{
    const int width = 238;
    const int height = 154;
    lv_obj_t *title;
    lv_obj_t *icon;
    const lv_image_dsc_t *asset =
        crazypod_menu_icon_asset(action_icon(actions.selected));

    actions.adjusting = true;
    begin_overlay(width, height);
    title = make_label(
        actions.panel, action_title(actions.selected),
        &lv_font_montserrat_16, COLOR_INK, LV_OPA_COVER);
    lv_obj_set_pos(title, 14, 12);
    lv_obj_set_width(title, width - 28);

    if(asset != NULL) {
        icon = lv_image_create(actions.panel);
        lv_image_set_src(icon, asset);
        lv_image_set_scale(
            icon, 21 * LV_SCALE_NONE / asset->header.w);
        lv_obj_set_style_image_recolor(
            icon, lv_color_hex(COLOR_INK), 0);
        lv_obj_set_style_image_recolor_opa(icon, LV_OPA_COVER, 0);
        lv_obj_set_pos(icon, 20, 53);
    }
    /* A horizontal, readable iOS-style level. Click Wheel remains the
     * only adjustment input: no touch slider or polling is introduced. */
    actions.level_bar = lv_bar_create(actions.panel);
    lv_obj_set_pos(actions.level_bar, 51, 60);
    lv_obj_set_size(actions.level_bar, width - 77, 12);
    lv_bar_set_range(actions.level_bar, 0, 100);
    lv_obj_remove_flag(actions.level_bar, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_radius(actions.level_bar, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(
        actions.level_bar, lv_color_hex(0xC8D2E0), 0);
    lv_obj_set_style_bg_opa(
        actions.level_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(
        actions.level_bar, lv_color_hex(COLOR_ACCENT),
        LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(
        actions.level_bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(
        actions.level_bar, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);

    actions.level_value = make_label(
        actions.panel, "", &lv_font_montserrat_24,
        COLOR_INK, LV_OPA_COVER);
    lv_obj_set_pos(actions.level_value, 14, 82);
    lv_obj_set_width(actions.level_value, width - 28);
    lv_obj_set_style_text_align(
        actions.level_value, LV_TEXT_ALIGN_CENTER, 0);
    actions.detail = make_label(
        actions.panel, CP_TR("Wheel adjusts"),
        &lv_font_montserrat_8, COLOR_MUTED, 210);
    lv_obj_set_pos(actions.detail, 10, 131);
    lv_obj_set_width(actions.detail, width - 20);
    lv_obj_set_style_text_align(
        actions.detail, LV_TEXT_ALIGN_CENTER, 0);
    refresh_adjustment();
}

static void move_selection(int direction)
{
    int next;

    if(direction == 0 || actions.adjusting)
        return;
    next = (int)actions.selected + (direction > 0 ? 1 : -1);
    if(next < 0)
        next = HOME_ACTION_COUNT - 1;
    if(next >= HOME_ACTION_COUNT)
        next = 0;
    actions.selected = (enum home_action)next;
    refresh_action_selection();
}

static void adjust_level(int direction)
{
    if(direction == 0 || !actions.adjusting)
        return;
    if(actions.selected == HOME_ACTION_BRIGHTNESS) {
        int next = global_settings.brightness + direction;

        if(next < MIN_BRIGHTNESS_SETTING)
            next = MIN_BRIGHTNESS_SETTING;
        if(next > MAX_BRIGHTNESS_SETTING)
            next = MAX_BRIGHTNESS_SETTING;
        if(next != global_settings.brightness) {
            global_settings.brightness = next;
            backlight_set_brightness(next);
            crazypod_state_mark_dirty();
        }
    }
    else if(actions.selected == HOME_ACTION_VOLUME) {
        int next = global_status.volume + direction * 2;

        if(next < sound_min(SOUND_VOLUME))
            next = sound_min(SOUND_VOLUME);
        if(next > sound_max(SOUND_VOLUME))
            next = sound_max(SOUND_VOLUME);
        if(next != global_status.volume) {
            sound_set_volume(next);
            global_status.volume = next;
            iap_on_volume(next);
            crazypod_state_mark_dirty();
        }
    }
    refresh_adjustment();
}

static void activate(void)
{
    if(actions.adjusting) {
        show_action_list();
        return;
    }
    if(actions.selected == HOME_ACTION_QUEUE) {
        void (*open_queue)(void) = actions.callbacks.open_queue;

        crazypod_home_actions_dismiss(true);
        if(open_queue != NULL)
            open_queue();
        return;
    }
    show_adjustment();
}

void crazypod_home_actions_configure(
    lv_obj_t *parent,
    const struct crazypod_home_actions_callbacks *callbacks)
{
    actions.parent = parent;
    if(callbacks != NULL)
        actions.callbacks = *callbacks;
}

void crazypod_home_actions_show(void)
{
    if(actions.parent == NULL || crazypod_home_actions_visible())
        return;
    actions.selected = HOME_ACTION_QUEUE;
    crazypod_desktop_native_prepare_modal();
    crazypod_overlay_glass_prepare(true);
    show_action_list();
}

void crazypod_home_actions_dismiss(bool restore_desktop)
{
    if(!crazypod_home_actions_visible())
        return;
    delete_overlay();
    actions.adjusting = false;
    if(restore_desktop)
        crazypod_desktop_native_restore_after_modal();
}

bool crazypod_home_actions_visible(void)
{
    return actions.root != NULL && lv_obj_is_valid(actions.root);
}

bool crazypod_home_actions_handle_button(
    long base, bool repeated, intptr_t data)
{
    int direction = 0;
    int step = 1;

    if(!crazypod_home_actions_visible())
        return false;
#ifdef HAVE_WHEEL_ACCELERATION
    step = button_apply_acceleration((unsigned int)data);
#else
    (void)data;
#endif
    if(step < 1)
        step = 1;
    if(step > 3)
        step = 3;
    if(base == BUTTON_SCROLL_FWD)
        direction = step;
    else if(base == BUTTON_SCROLL_BACK)
        direction = -step;
    if(direction != 0) {
        if(actions.adjusting)
            adjust_level(direction);
        else
            move_selection(direction);
    }
    else if(base == BUTTON_SELECT && !repeated)
        activate();
    else if(base == BUTTON_MENU && !repeated)
        crazypod_home_actions_dismiss(true);
    else if(base == BUTTON_PLAY && !repeated &&
            actions.callbacks.toggle_playback != NULL)
        actions.callbacks.toggle_playback();
    return true;
}

#endif
