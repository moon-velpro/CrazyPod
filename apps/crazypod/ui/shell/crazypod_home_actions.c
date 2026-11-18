#include "config.h"

#include "../../crazypod_l10n.h"

#ifdef IPOD_6G

#include <stdio.h>

#include "backlight.h"
#include "iap-usb.h"
#include "settings.h"
#include "sound.h"

#include "../../crazypod_state.h"
#include "../presentation/crazypod_ui_widgets.h"
#include "crazypod_home_actions.h"

#define HOME_ACTION_COUNT 3
#define HOME_BG 0xF6F9FC
#define HOME_TEXT 0x1E2938
#define HOME_MUTED 0x64748B
#define HOME_ACCENT 0x447AA9
#define HOME_TRACK 0xDDE5ED

bool crazypod_home_actions_is_route(enum crazypod_route route)
{
    return route == HOME_ROUTE_ACTIONS ||
           route == HOME_ROUTE_BRIGHTNESS ||
           route == HOME_ROUTE_VOLUME;
}

bool crazypod_home_actions_is_adjustment(enum crazypod_route route)
{
    return route == HOME_ROUTE_BRIGHTNESS ||
           route == HOME_ROUTE_VOLUME;
}

int crazypod_home_actions_item_count(enum crazypod_route route)
{
    return route == HOME_ROUTE_ACTIONS ? HOME_ACTION_COUNT : 0;
}

const char *crazypod_home_actions_item_title(int index)
{
    switch(index) {
    case 0: return CP_TR("Playback Queue");
    case 1: return CP_TR("Brightness");
    case 2: return CP_TR("VOLUME");
    }
    return "";
}

const char *crazypod_home_actions_title(enum crazypod_route route)
{
    switch(route) {
    case HOME_ROUTE_ACTIONS: return CP_TR("ACTIONS");
    case HOME_ROUTE_BRIGHTNESS: return CP_TR("Brightness");
    case HOME_ROUTE_VOLUME: return CP_TR("VOLUME");
    default: return "";
    }
}

static int level_percent(enum crazypod_route route)
{
    int value;
    int minimum;
    int maximum;

    if(route == HOME_ROUTE_BRIGHTNESS) {
        value = global_settings.brightness;
        minimum = MIN_BRIGHTNESS_SETTING;
        maximum = MAX_BRIGHTNESS_SETTING;
    }
    else if(route == HOME_ROUTE_VOLUME) {
        value = global_status.volume;
        minimum = sound_min(SOUND_VOLUME);
        maximum = sound_max(SOUND_VOLUME);
    }
    else
        return 0;

    if(maximum <= minimum)
        return 0;
    if(value < minimum)
        value = minimum;
    if(value > maximum)
        value = maximum;
    return (value - minimum) * 100 / (maximum - minimum);
}

bool crazypod_home_actions_adjust(
    enum crazypod_route route, int direction)
{
    int value;
    int minimum;
    int maximum;

    if(direction == 0 || !crazypod_home_actions_is_adjustment(route))
        return false;
    if(route == HOME_ROUTE_BRIGHTNESS) {
        value = global_settings.brightness;
        minimum = MIN_BRIGHTNESS_SETTING;
        maximum = MAX_BRIGHTNESS_SETTING;
    }
    else {
        value = global_status.volume;
        minimum = sound_min(SOUND_VOLUME);
        maximum = sound_max(SOUND_VOLUME);
    }
    value += route == HOME_ROUTE_VOLUME ? direction * 2 : direction;
    if(value < minimum)
        value = minimum;
    if(value > maximum)
        value = maximum;

    if(route == HOME_ROUTE_BRIGHTNESS) {
        if(value == global_settings.brightness)
            return false;
        global_settings.brightness = value;
        backlight_set_brightness(value);
    }
    else {
        if(value == global_status.volume)
            return false;
        global_status.volume = value;
        sound_set_volume(value);
        iap_on_volume(value);
    }
    crazypod_state_mark_dirty();
    return true;
}

void crazypod_home_actions_render_adjustment(
    enum crazypod_route route, lv_obj_t *parent)
{
    char value_text[12];
    int percent;
    int width;
    lv_obj_t *label;

    if(parent == NULL || !crazypod_home_actions_is_adjustment(route))
        return;
    percent = level_percent(route);
    width = (252 * percent) / 100;
    snprintf(value_text, sizeof(value_text), "%d%%", percent);
    (void)crazypod_ui_widget_box(
        parent, 0, 32, 320, 208, 0, HOME_BG, LV_OPA_COVER);

    label = crazypod_ui_widget_label(
        parent, crazypod_home_actions_title(route),
        &lv_font_montserrat_16, HOME_TEXT, LV_OPA_COVER);
    lv_obj_set_pos(label, 16, 45);
    lv_obj_set_width(label, 288);

    label = crazypod_ui_widget_label(
        parent, value_text, &lv_font_montserrat_24,
        HOME_TEXT, LV_OPA_COVER);
    lv_obj_set_pos(label, 25, 103);
    lv_obj_set_width(label, 270);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

    (void)crazypod_ui_widget_box(
        parent, 34, 158, 252, 8, 4, HOME_TRACK, LV_OPA_COVER);
    if(width > 0)
        (void)crazypod_ui_widget_box(
            parent, 34, 158, width, 8, 4,
            HOME_ACCENT, LV_OPA_COVER);

    label = crazypod_ui_widget_label(
        parent, CP_TR("Wheel adjusts"), &lv_font_montserrat_12,
        HOME_MUTED, LV_OPA_COVER);
    lv_obj_set_pos(label, 16, 193);
    lv_obj_set_width(label, 288);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_DOTS);
}

#endif
