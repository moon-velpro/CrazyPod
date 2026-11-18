#include "config.h"

#include "../../crazypod_l10n.h"

#ifdef IPOD_6G

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
#define HOME_PAGE 0xF6F9FC
#define HOME_TEXT 0x1E2938
#define HOME_MUTED 0x64748B
#define HOME_ACCENT 0x447AA9
#define HOME_SELECTED 0xDDEAF7
#define HOME_VISIBLE_ROWS 4
#define HOME_ROW_HEIGHT 29
#define HOME_ROW_STEP 30
#define HOME_ROW_Y 53
#define HOME_WHEEL_POSITIONS 96
#define HOME_WHEEL_CLICKS_PER_DETENT 8
#define HOME_WHEEL_RELEASE_TICKS \
    (((HZ * 6) / 100) > 0 ? ((HZ * 6) / 100) : 1)

static struct crazypod_desktop_host desktop_host;
static lv_obj_t *screen;
static lv_obj_t *wallpaper;
static lv_obj_t *title;
static lv_obj_t *home_rows[HOME_VISIBLE_ROWS];
static lv_obj_t *home_labels[HOME_VISIBLE_ROWS];
static lv_obj_t *home_markers[HOME_VISIBLE_ROWS];
static lv_obj_t *home_scroll_thumb;
static int selected_app;
static long wheel_last_seen;
static int wheel_position;
static int wheel_detent_accumulator;
static int wheel_feedback_direction;
static bool desktop_active;
static bool wheel_tracking;
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

static const struct crazypod_app_descriptor *visible_app(int index)
{
    return crazypod_app_catalog_find(crazypod_apps_visible_id(index));
}

/* An iPod menu, not a horizontal app launcher. */
static void update_selection_chrome(void)
{
    int count = crazypod_apps_visible_count();
    int start = selected_app - HOME_VISIBLE_ROWS / 2;
    int row;

    if(start < 0)
        start = 0;
    if(start > count - HOME_VISIBLE_ROWS)
        start = count - HOME_VISIBLE_ROWS;
    if(start < 0)
        start = 0;
    for(row = 0; row < HOME_VISIBLE_ROWS; ++row) {
        int index = start + row;
        const struct crazypod_app_descriptor *app =
            index < count ? visible_app(index) : NULL;
        bool selected = app != NULL && index == selected_app;

        if(home_rows[row] == NULL)
            continue;
        if(app == NULL) {
            lv_obj_add_flag(home_rows[row], LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        lv_obj_remove_flag(home_rows[row], LV_OBJ_FLAG_HIDDEN);
        CP_LV_LABEL_SET_TEXT(home_labels[row], app->name);
        lv_obj_set_style_bg_color(
            home_rows[row],
            lv_color_hex(selected ? HOME_SELECTED : COLOR_WHITE), 0);
        lv_obj_set_style_bg_opa(home_rows[row], LV_OPA_COVER, 0);
        lv_obj_set_style_text_opa(
            home_labels[row], selected ? 255 : 225, 0);
        lv_obj_set_style_text_opa(
            home_markers[row], selected ? 255 : 145, 0);
    }
    if(home_scroll_thumb != NULL) {
        int height = count > HOME_VISIBLE_ROWS
            ? 112 * HOME_VISIBLE_ROWS / count : 112;
        int y = 56;
        if(height < 12)
            height = 12;
        if(count > 1)
            y += (112 - height) * selected_app / (count - 1);
        lv_obj_set_y(home_scroll_thumb, y);
        lv_obj_set_height(home_scroll_thumb, height);
        if(count <= HOME_VISIBLE_ROWS)
            lv_obj_add_flag(
                home_scroll_thumb, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_remove_flag(
                home_scroll_thumb, LV_OBJ_FLAG_HIDDEN);
    }
}

static void layout(void)
{
    update_selection_chrome();
}

static int clamp_selected(int selected)
{
    int count = crazypod_apps_visible_count();

    if(selected < 0)
        return 0;
    if(selected >= count)
        return count > 0 ? count - 1 : 0;
    return selected;
}

#ifdef HAVE_WHEEL_POSITION
static bool set_selected_target(int selected)
{
    selected = clamp_selected(selected);

    if(selected == selected_app)
        return false;
    selected_app = selected;
    update_selection_chrome();
    return true;
}
#endif

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
    }
}
#endif

lv_obj_t *crazypod_desktop_create(
    long now, const lv_font_t *metadata_font,
    const struct crazypod_desktop_host *host)
{
    const lv_image_dsc_t *image =
        crazypod_custom_home_wallpaper();
    int i;

    memset(&desktop_host, 0, sizeof(desktop_host));
    if(host != NULL)
        desktop_host = *host;
    (void)now;
    screen = lv_obj_create(NULL);
    crazypod_ui_widget_make_plain(screen);
    lv_obj_set_style_bg_color(
        screen, lv_color_hex(HOME_PAGE), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    /* Never substitute a colorful default wallpaper for the menu. */
    wallpaper = lv_image_create(screen);
    if(image != NULL)
        lv_image_set_src(wallpaper, image);
    else
        lv_obj_add_flag(wallpaper, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(wallpaper, 0, 0);
    lv_obj_remove_flag(wallpaper, LV_OBJ_FLAG_CLICKABLE);

    crazypod_status_bar_create(0, screen);
    crazypod_status_bar_set_palette(0, HOME_TEXT, HOME_PAGE);
    title = crazypod_ui_widget_label(
        screen, CP_TR("Main Menu"), &lv_font_montserrat_16,
        HOME_TEXT, LV_OPA_COVER);
    lv_obj_set_pos(title, 14, 33);
    lv_obj_set_width(title, 292);

    for(i = 0; i < HOME_VISIBLE_ROWS; ++i) {
        home_rows[i] = crazypod_ui_widget_box(
            screen, 8, HOME_ROW_Y + i * HOME_ROW_STEP,
            304, HOME_ROW_HEIGHT, 7,
            COLOR_WHITE, LV_OPA_COVER);
        home_labels[i] = crazypod_ui_widget_label(
            home_rows[i], "", metadata_font, HOME_TEXT, 230);
        lv_obj_set_pos(home_labels[i], 14, 3);
        lv_obj_set_width(home_labels[i], 254);
        lv_label_set_long_mode(
            home_labels[i], LV_LABEL_LONG_MODE_DOTS);
        home_markers[i] = crazypod_ui_widget_label(
            home_rows[i], LV_SYMBOL_RIGHT,
            &lv_font_montserrat_10, HOME_ACCENT, 150);
        lv_obj_set_pos(home_markers[i], 279, 8);
        lv_obj_set_width(home_markers[i], 18);
    }
    home_scroll_thumb = crazypod_ui_widget_box(
        screen, 315, 56, 2, 26, 1,
        HOME_ACCENT, 195);

    crazypod_now_capsule_create(screen, metadata_font);
    selected_app = 0;
    wheel_last_seen = now;
    wheel_position = -1;
    wheel_detent_accumulator = 0;
    wheel_feedback_direction = 0;
    desktop_active = true;
    wheel_tracking = false;
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
    /* The list uses LVGL dirty rectangles; no invisible carousel spring. */
    (void)animated;
    selected_app = clamp_selected(selected);
    wheel_tracking = false;
    wheel_position = -1;
    wheel_detent_accumulator = 0;
    layout();
}

void crazypod_desktop_move_selection(int direction)
{
    int count = crazypod_apps_visible_count();
    int selected = selected_app + direction;

    if(count <= 1 || direction == 0 ||
       selected < 0 || selected >= count ||
       selected == selected_app)
        return;
    crazypod_desktop_set_selected(selected, true);
}

void crazypod_desktop_set_active(bool active, long now)
{
    (void)now;
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
}

bool crazypod_desktop_motion_active(void)
{
    return desktop_active && wheel_tracking;
}

bool crazypod_desktop_wheel_touch_active(void)
{
#ifdef HAVE_WHEEL_POSITION
    return desktop_active && wheel_touch_status();
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

    lv_obj_set_style_bg_color(
        screen, lv_color_hex(HOME_PAGE), 0);
    crazypod_status_bar_set_palette(0, HOME_TEXT, HOME_PAGE);
    crazypod_icons_load_theme(crazypod_appearance_get()->icon_theme);
    crazypod_now_capsule_refresh_material();
    crazypod_now_capsule_refresh_appearance();
    layout();
    if(desktop_host.refresh_corner_masks != NULL)
        desktop_host.refresh_corner_masks();
    if(desktop_host.refresh_lock_appearance != NULL)
        desktop_host.refresh_lock_appearance();
    lv_obj_invalidate(screen);
}

/*
 * The original native icon carousel is retired. Keep the entrypoints so
 * existing hardware frame scheduling remains ABI-compatible.
 */
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
