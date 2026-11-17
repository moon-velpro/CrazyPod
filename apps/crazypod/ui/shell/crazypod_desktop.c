#include "config.h"

#include "../../crazypod_l10n.h"

#ifdef IPOD_6G

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "button.h"
#include "kernel.h"

#include "../../crazypod_appearance.h"
#include "../../crazypod_apps.h"
#include "../../crazypod_frameclock.h"
#include "../../crazypod_icons.h"
#include "../../crazypod_wallpaper.h"
#include "../presentation/crazypod_hold_feedback.h"
#include "../presentation/crazypod_ui_widgets.h"
#include "crazypod_app_catalog.h"
#include "crazypod_desktop.h"
#include "crazypod_desktop_native.h"
#include "crazypod_now_capsule.h"
#include "crazypod_status_bar.h"

#define COLOR_WHITE 0xFFFFFF
#define HOME_INK 0x101217
#define HOME_ACCENT 0xE9E2D7
#define HOME_ENTRY_COUNT 4
#define HOME_ROW_X 15
#define HOME_ROW_TOP 36
#define HOME_ROW_HEIGHT 30
#define HOME_ROW_GAP 4
#define HOME_POSITION_ONE (1L << 16)
#define HOME_WHEEL_POSITIONS 96
#define HOME_WHEEL_CLICKS_PER_DETENT 8
#define HOME_WHEEL_RELEASE_TICKS \
    (((HZ * 6) / 100) > 0 ? ((HZ * 6) / 100) : 1)
#define HOME_SPRING_STIFFNESS 503
#define HOME_SPRING_DAMPING 37
#define HOME_SPRING_POSITION_EPSILON (HOME_POSITION_ONE / 1024)
#define HOME_SPRING_VELOCITY_EPSILON (HOME_POSITION_ONE / 64)
#define HOME_INDICATOR_WIDTH 5
#define HOME_SELECTED_INDICATOR_WIDTH 14
#define HOME_INDICATOR_GAP 4

static struct crazypod_desktop_host desktop_host;
static lv_obj_t *screen;
static lv_obj_t *wallpaper;
static lv_obj_t *home_rows[HOME_ENTRY_COUNT];
static lv_obj_t *home_labels[HOME_ENTRY_COUNT];
static lv_obj_t *preview_title;
static lv_obj_t *preview_index;
static int selected_app;
static int32_t position_q16;
static int32_t spring_velocity_q16;
static long motion_last_tick;
static long wheel_last_seen;
static int wheel_position;
static int wheel_detent_accumulator;
static int wheel_feedback_direction;
static bool desktop_active;
static bool wheel_tracking;
static bool springing;
static lv_obj_t *hold_feedback_root;
static bool hold_feedback_teardown_pending;
static struct crazypod_hold_feedback hold_feedback;

static void hold_feedback_teardown_ready(lv_event_t *event)
{
    lv_display_t *display = lv_event_get_target(event);

    lv_display_remove_event_cb_with_user_data(
        display, hold_feedback_teardown_ready, NULL);
    if(!hold_feedback_teardown_pending)
        return;
    hold_feedback_teardown_pending = false;
    crazypod_desktop_native_restore_after_modal();
}

/* Product Home stays independent of the persisted experimental app order. */
static const char *home_entry_title(int index)
{
    switch(index) {
    case 0: return CP_TR("Now Playing");
    case 1: return CP_TR("Music");
    case 2: return CP_TR("Podcasts");
    case 3: return CP_TR("Settings");
    default: return "";
    }
}

static void update_selection_chrome(void)
{
    char index_text[12];
    int index;

    for(index = 0; index < HOME_ENTRY_COUNT; ++index) {
        bool selected = index == selected_app;

        if(home_rows[index] == NULL || home_labels[index] == NULL)
            continue;
        /* Rebind localized labels after a runtime language change. */
        CP_LV_LABEL_SET_TEXT(home_labels[index], home_entry_title(index));
        lv_obj_set_style_bg_color(
            home_rows[index], lv_color_hex(HOME_ACCENT), 0);
        lv_obj_set_style_bg_opa(
            home_rows[index], selected ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        lv_obj_set_style_text_color(
            home_labels[index],
            lv_color_hex(selected ? HOME_INK : COLOR_WHITE), 0);
        lv_obj_set_style_opa(
            home_labels[index], selected ? 255 : 205, 0);
    }
    if(preview_title != NULL)
        CP_LV_LABEL_SET_TEXT(preview_title, home_entry_title(selected_app));
    if(preview_index != NULL) {
        snprintf(index_text, sizeof(index_text),
                 "%02d / %02d", selected_app + 1, HOME_ENTRY_COUNT);
        CP_LV_LABEL_SET_TEXT(preview_index, index_text);
    }
}

static void layout(void)
{
    update_selection_chrome();
    if(screen != NULL)
        lv_obj_invalidate(screen);
}

static int clamp_selected(int selected)
{
    int count = HOME_ENTRY_COUNT;

    if(selected < 0)
        return 0;
    if(selected >= count)
        return count > 0 ? count - 1 : 0;
    return selected;
}

static int32_t selected_position_q16(void)
{
    return selected_app * HOME_POSITION_ONE;
}

static void start_spring(void)
{
    springing = position_q16 != selected_position_q16() ||
        spring_velocity_q16 != 0;
    if(springing)
        crazypod_desktop_native_invalidate(false);
}

#ifdef HAVE_WHEEL_POSITION
static bool set_selected_target(int selected)
{
    selected = clamp_selected(selected);

    if(selected == selected_app)
        return false;
    selected_app = selected;
    update_selection_chrome();
    start_spring();
    return true;
}
#endif

static void advance_spring_tick(void)
{
    int32_t target = selected_position_q16();
    int64_t acceleration;
    int32_t maximum = (HOME_ENTRY_COUNT - 1) * HOME_POSITION_ONE;

    if(!springing)
        return;
    acceleration =
        (int64_t)(target - position_q16) * HOME_SPRING_STIFFNESS -
        (int64_t)spring_velocity_q16 * HOME_SPRING_DAMPING;
    spring_velocity_q16 += (int32_t)(acceleration / HZ);
    position_q16 += spring_velocity_q16 / HZ;
    if(position_q16 < 0) {
        position_q16 = 0;
        if(spring_velocity_q16 < 0)
            spring_velocity_q16 = 0;
    }
    else if(position_q16 > maximum) {
        position_q16 = maximum;
        if(spring_velocity_q16 > 0)
            spring_velocity_q16 = 0;
    }
    if(position_q16 >= target - HOME_SPRING_POSITION_EPSILON &&
       position_q16 <= target + HOME_SPRING_POSITION_EPSILON &&
       spring_velocity_q16 >= -HOME_SPRING_VELOCITY_EPSILON &&
       spring_velocity_q16 <= HOME_SPRING_VELOCITY_EPSILON) {
        position_q16 = target;
        spring_velocity_q16 = 0;
        springing = false;
    }
}

#ifdef HAVE_WHEEL_POSITION
static void apply_wheel_clicks(int delta)
{
    int direction;

    wheel_detent_accumulator += delta;
    while(wheel_detent_accumulator >= HOME_WHEEL_CLICKS_PER_DETENT ||
          wheel_detent_accumulator <= -HOME_WHEEL_CLICKS_PER_DETENT) {
        direction = wheel_detent_accumulator < 0 ? -1 : 1;
        if(!set_selected_target(selected_app + direction)) {
            wheel_detent_accumulator = 0;
            if((direction < 0 && spring_velocity_q16 < 0) ||
               (direction > 0 && spring_velocity_q16 > 0))
                spring_velocity_q16 = 0;
            return;
        }
        wheel_detent_accumulator -=
            direction * HOME_WHEEL_CLICKS_PER_DETENT;
        wheel_feedback_direction = direction;
    }
}

static void sample_wheel(long now)
{
    int current = wheel_status();

    if(current >= 0) {
        current %= HOME_WHEEL_POSITIONS;
        if(!wheel_tracking) {
            wheel_tracking = true;
            wheel_position = current;
            wheel_detent_accumulator = 0;
            motion_last_tick = now;
        }
        else {
            int delta = current - wheel_position;

            if(delta < -HOME_WHEEL_POSITIONS / 2)
                delta += HOME_WHEEL_POSITIONS;
            else if(delta > HOME_WHEEL_POSITIONS / 2)
                delta -= HOME_WHEEL_POSITIONS;
            wheel_position = current;
            if(delta != 0)
                apply_wheel_clicks(delta);
        }
        wheel_last_seen = now;
        return;
    }
    if(wheel_tracking &&
       !TIME_BEFORE(now,
                    wheel_last_seen + HOME_WHEEL_RELEASE_TICKS)) {
        wheel_tracking = false;
        wheel_position = -1;
        wheel_detent_accumulator = 0;
        motion_last_tick = now;
    }
}
#endif

static void advance_motion(long now)
{
    long elapsed = now - motion_last_tick;
    long steps;

    if(elapsed <= 0)
        return;
    motion_last_tick = now;
    if(elapsed > HZ / 2) {
        position_q16 = selected_position_q16();
        spring_velocity_q16 = 0;
        springing = false;
        return;
    }
    for(steps = 0; steps < elapsed; ++steps)
        advance_spring_tick();
    if(springing)
        crazypod_desktop_native_invalidate(false);
}

lv_obj_t *crazypod_desktop_create(
    long now, const lv_font_t *metadata_font,
    const struct crazypod_desktop_host *host)
{
    const lv_image_dsc_t *image = crazypod_custom_home_wallpaper();
    const lv_font_t *body_font = metadata_font != NULL
        ? metadata_font : &lv_font_montserrat_14;
    lv_obj_t *separator;
    lv_obj_t *kicker;
    int i;

    memset(&desktop_host, 0, sizeof(desktop_host));
    if(host != NULL)
        desktop_host = *host;
    screen = lv_obj_create(NULL);
    crazypod_ui_widget_make_plain(screen);
    lv_obj_set_style_bg_color(screen, lv_color_hex(HOME_INK), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    wallpaper = lv_image_create(screen);
    if(image != NULL)
        lv_image_set_src(wallpaper, image);
    else
        lv_obj_add_flag(wallpaper, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(wallpaper, 0, 0);
    lv_obj_remove_flag(wallpaper, LV_OBJ_FLAG_CLICKABLE);
    crazypod_status_bar_create(0, screen);

    /* Four legible destinations, with no app-carousel icon decoding. */
    for(i = 0; i < HOME_ENTRY_COUNT; ++i) {
        home_rows[i] = crazypod_ui_widget_box(
            screen, HOME_ROW_X,
            HOME_ROW_TOP + i * (HOME_ROW_HEIGHT + HOME_ROW_GAP),
            152, HOME_ROW_HEIGHT, 8, HOME_ACCENT, LV_OPA_TRANSP);
        home_labels[i] = crazypod_ui_widget_label(
            home_rows[i], home_entry_title(i),
            body_font, COLOR_WHITE, LV_OPA_COVER);
        lv_obj_set_pos(home_labels[i], 11, 5);
        lv_obj_set_width(home_labels[i], 128);
        lv_label_set_long_mode(home_labels[i], LV_LABEL_LONG_MODE_DOTS);
    }
    separator = crazypod_ui_widget_box(
        screen, 175, 40, 1, 120, 0, COLOR_WHITE, 40);
    lv_obj_remove_flag(separator, LV_OBJ_FLAG_CLICKABLE);
    kicker = crazypod_ui_widget_label(
        screen, "CrazyPod", &lv_font_montserrat_12,
        HOME_ACCENT, 155);
    lv_obj_set_pos(kicker, 187, 45);
    preview_title = crazypod_ui_widget_label(
        screen, home_entry_title(0), body_font, COLOR_WHITE, LV_OPA_COVER);
    lv_obj_set_pos(preview_title, 187, 90);
    lv_obj_set_width(preview_title, 123);
    lv_label_set_long_mode(preview_title, LV_LABEL_LONG_MODE_DOTS);
    preview_index = crazypod_ui_widget_label(
        screen, "01 / 04", &lv_font_montserrat_12,
        HOME_ACCENT, 155);
    lv_obj_set_pos(preview_index, 187, 139);

    crazypod_now_capsule_create(screen, metadata_font);
    selected_app = 0;
    position_q16 = 0;
    spring_velocity_q16 = 0;
    motion_last_tick = now;
    wheel_last_seen = now;
    wheel_position = -1;
    wheel_detent_accumulator = 0;
    wheel_feedback_direction = 0;
    desktop_active = true;
    wheel_tracking = false;
    springing = false;
#ifdef HAVE_WHEEL_POSITION
    wheel_send_events(false);
#endif
    crazypod_desktop_native_reset();
    layout();
    if(desktop_host.create_corner_masks != NULL)
        desktop_host.create_corner_masks(screen, 0);
    return screen;
}

lv_obj_t *crazypod_desktop_screen(void)
{
    return screen;
}

int crazypod_desktop_selected(void)
{
    return selected_app;
}

void crazypod_desktop_set_selected(int selected, bool animated)
{
    selected = clamp_selected(selected);
    wheel_tracking = false;
    wheel_detent_accumulator = 0;
    motion_last_tick = current_tick;
    if(animated) {
        if(selected != selected_app) {
            selected_app = selected;
            update_selection_chrome();
        }
        start_spring();
    }
    else {
        selected_app = selected;
        position_q16 = selected * HOME_POSITION_ONE;
        spring_velocity_q16 = 0;
        springing = false;
        layout();
    }
}

void crazypod_desktop_move_selection(int direction)
{
    int count = HOME_ENTRY_COUNT;
    int selected = selected_app + direction;

    if(count <= 1 || direction == 0 ||
       selected < 0 || selected >= count ||
       selected == selected_app)
        return;
    crazypod_desktop_set_selected(selected, true);
}

void crazypod_desktop_set_active(bool active, long now)
{
    desktop_active = active;
#ifdef HAVE_WHEEL_POSITION
    wheel_send_events(!active);
#endif
    if(active)
        return;
    wheel_tracking = false;
    wheel_position = -1;
    wheel_detent_accumulator = 0;
    wheel_feedback_direction = 0;
    advance_motion(now);
    position_q16 = selected_app * HOME_POSITION_ONE;
    spring_velocity_q16 = 0;
    springing = false;
}

void crazypod_desktop_tick(long now)
{
    if(!desktop_active)
        return;
#ifdef HAVE_WHEEL_POSITION
    sample_wheel(now);
#else
    (void)now;
#endif
    advance_motion(now);
}

bool crazypod_desktop_motion_active(void)
{
    return desktop_active &&
        (wheel_tracking || springing);
}

bool crazypod_desktop_wheel_touch_active(void)
{
#ifdef HAVE_WHEEL_POSITION
    /* This Home is LVGL-owned; native-only present deferral must stay off. */
    return false;
#else
    return false;
#endif
}

int crazypod_desktop_take_wheel_feedback(void)
{
    int direction = wheel_feedback_direction;

    wheel_feedback_direction = 0;
    return direction;
}

bool crazypod_desktop_hold_feedback_visible(void)
{
    return hold_feedback_root != NULL ||
        hold_feedback_teardown_pending;
}

void crazypod_desktop_hold_feedback_dismiss(
    bool preserve_underlay)
{
    lv_obj_t *root = hold_feedback_root;
    lv_display_t *display;

    if(root == NULL)
        return;
    if(preserve_underlay)
        crazypod_desktop_native_preserve_modal_underlay();
    crazypod_hold_feedback_dismiss(&hold_feedback);
    hold_feedback_root = NULL;
    hold_feedback_teardown_pending = true;
    display = lv_obj_get_display(root);
    lv_display_remove_event_cb_with_user_data(
        display, hold_feedback_teardown_ready, NULL);
    lv_display_add_event_cb(
        display, hold_feedback_teardown_ready,
        LV_EVENT_REFR_READY, NULL);
    lv_obj_delete(root);
}

void crazypod_desktop_hold_feedback_begin(
    const char *symbol, int duration_ms)
{
    if(!desktop_active ||
       screen == NULL || !lv_obj_is_valid(screen) ||
       crazypod_desktop_hold_feedback_visible())
        return;
    crazypod_desktop_native_prepare_modal();
    hold_feedback_root = crazypod_ui_widget_box(
        screen, 0, 0, LCD_WIDTH, LCD_HEIGHT,
        0, 0x000000, LV_OPA_TRANSP);
    lv_obj_remove_flag(
        hold_feedback_root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_move_foreground(hold_feedback_root);
    (void)crazypod_desktop_native_create_modal_underlay(
        hold_feedback_root);
    crazypod_hold_feedback_begin_topbar(
        &hold_feedback, hold_feedback_root,
        symbol, duration_ms);
    if(hold_feedback.root == NULL)
        crazypod_desktop_hold_feedback_dismiss(false);
}

void crazypod_desktop_refresh_appearance(void)
{
    const lv_image_dsc_t *custom = crazypod_custom_home_wallpaper();

    if(screen == NULL)
        return;
    if(custom != NULL) {
        lv_image_set_src(wallpaper, custom);
        lv_obj_remove_flag(wallpaper, LV_OBJ_FLAG_HIDDEN);
    }
    else
        lv_obj_add_flag(wallpaper, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(screen, lv_color_hex(HOME_INK), 0);
    crazypod_now_capsule_refresh_material();
    crazypod_now_capsule_refresh_appearance();
    layout();
    if(desktop_host.refresh_corner_masks != NULL)
        desktop_host.refresh_corner_masks();
    if(desktop_host.refresh_lock_appearance != NULL)
        desktop_host.refresh_lock_appearance();
    lv_obj_invalidate(screen);
}

/* The original native icon carousel remains in Git history for Lab. */
void crazypod_desktop_render_icon(int tile_size, bool blocked)
{
    (void)tile_size;
    (void)blocked;
}

void crazypod_desktop_render_icon_snapshot(int tile_size)
{
    (void)tile_size;
}

#endif
