#include "config.h"

#include "../../crazypod_l10n.h"

#ifdef IPOD_6G

#include <string.h>

#include "button.h"
#include "kernel.h"

#include "../../crazypod_frameclock.h"
#include "../../crazypod_wallpaper.h"
#include "../../crazypod_product_geometry.h"
#include "../presentation/crazypod_hold_feedback.h"
#include "../presentation/crazypod_liquid_music.h"
#include "../presentation/crazypod_ui_widgets.h"
#include "crazypod_desktop.h"
#include "crazypod_desktop_native.h"
#include "crazypod_home_model.h"
#include "crazypod_now_capsule.h"
#include "crazypod_status_bar.h"

#define COLOR_WHITE 0xFFFFFF
#define HOME_INK CRAZYPOD_LM_HOME_TOP
#define HOME_GLASS CRAZYPOD_LM_HOME_GLASS
#define HOME_ROW_X CRAZYPOD_PRODUCT_HOME_ROW_X
#define HOME_ROW_TOP CRAZYPOD_PRODUCT_HOME_ROW_TOP
#define HOME_ROW_WIDTH (LCD_WIDTH - 2 * HOME_ROW_X)
#define HOME_ROW_HEIGHT CRAZYPOD_PRODUCT_HOME_ROW_HEIGHT
#define HOME_ROW_GAP CRAZYPOD_PRODUCT_HOME_ROW_GAP
#define HOME_WHEEL_POSITIONS 96
#define HOME_WHEEL_CLICKS_PER_DETENT 8
#define HOME_WHEEL_RELEASE_TICKS \
    (((HZ * 6) / 100) > 0 ? ((HZ * 6) / 100) : 1)

static struct crazypod_desktop_host desktop_host;
static lv_obj_t *screen;
static lv_obj_t *wallpaper;
static lv_obj_t *wallpaper_dim;
static lv_obj_t *home_headline;
static lv_obj_t *home_badges[CRAZYPOD_HOME_DESTINATION_COUNT];
static lv_obj_t *home_symbols[CRAZYPOD_HOME_DESTINATION_COUNT];
static lv_obj_t *home_chevrons[CRAZYPOD_HOME_DESTINATION_COUNT];
static lv_obj_t *home_rows[CRAZYPOD_HOME_DESTINATION_COUNT];
static lv_obj_t *home_labels[CRAZYPOD_HOME_DESTINATION_COUNT];
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

/* Product Home stays independent of the persisted experimental app order. */
static const char *home_entry_title(int index)
{
    switch(index) {
    case CRAZYPOD_HOME_NOW_PLAYING: return CP_TR("Now Playing");
    case CRAZYPOD_HOME_MUSIC: return CP_TR("Music");
    case CRAZYPOD_HOME_PODCASTS: return CP_TR("Podcasts");
    case CRAZYPOD_HOME_SETTINGS: return CP_TR("Settings");
    default: return "";
    }
}

static const char *home_entry_symbol(int index)
{
    switch(index) {
    case CRAZYPOD_HOME_NOW_PLAYING: return LV_SYMBOL_PLAY;
    case CRAZYPOD_HOME_MUSIC: return LV_SYMBOL_AUDIO;
    case CRAZYPOD_HOME_PODCASTS: return LV_SYMBOL_VOLUME_MID;
    case CRAZYPOD_HOME_SETTINGS: return LV_SYMBOL_SETTINGS;
    default: return LV_SYMBOL_RIGHT;
    }
}

static uint32_t home_entry_color(int index)
{
    static const uint32_t colors[] = {
        0x527FC0, 0xF24E82, 0x855ED5, 0x63758B
    };

    return index >= 0 && index < CRAZYPOD_HOME_DESTINATION_COUNT
        ? colors[index] : colors[0];
}

/* A four-row LVGL Home has no native moving band. Request a full,
 * hardware-synchronized present only when selection changes while the
 * physical wheel is touched. Keep unrelated partial LVGL work deferred until
 * release, preserving the Home interaction gate and avoiding unsynced DMA. */
static void queue_touched_home_selection(void)
{
#ifdef HAVE_WHEEL_POSITION
    if(desktop_active && wheel_touch_status())
        crazypod_present_queue_full();
#endif
}

static void update_selection_row(int index)
{
    bool selected = index == selected_app;

    if(home_rows[index] == NULL || home_labels[index] == NULL)
        return;
        /* A cool, quiet white material with a dark-ink hierarchy.
         * No continuous blur/refraction work on the ARM5 playback path. */
        lv_obj_set_style_bg_color(
            home_rows[index], lv_color_hex(HOME_GLASS), 0);
        lv_obj_set_style_bg_opa(
            home_rows[index], selected
                ? CRAZYPOD_LM_HOME_SELECTED_OPA :
                  CRAZYPOD_LM_HOME_IDLE_OPA, 0);
        lv_obj_set_style_border_width(home_rows[index], 1, 0);
        lv_obj_set_style_border_color(
            home_rows[index], lv_color_hex(COLOR_WHITE), 0);
        lv_obj_set_style_border_opa(
            home_rows[index], selected
                ? CRAZYPOD_LM_HOME_RIM_OPA : 96, 0);
        lv_obj_set_style_text_color(
            home_labels[index],
            lv_color_hex(selected
                ? CRAZYPOD_LM_HOME_TEXT :
                  CRAZYPOD_LM_HOME_MUTED), 0);
        lv_obj_set_style_opa(home_labels[index], LV_OPA_COVER, 0);
        if(home_badges[index] != NULL)
            lv_obj_set_style_opa(
                home_badges[index], selected ? 255 : 229, 0);
        if(home_chevrons[index] != NULL)
            lv_obj_set_style_opa(
                home_chevrons[index], selected ? 190 : 110, 0);
}

static void update_selection_chrome(void)
{
    int index;

    for(index = 0; index < CRAZYPOD_HOME_DESTINATION_COUNT; ++index) {
        CP_LV_LABEL_SET_TEXT(home_labels[index], home_entry_title(index));
        update_selection_row(index);
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
    int count = CRAZYPOD_HOME_DESTINATION_COUNT;

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
    {
        int previous = selected_app;
        selected_app = selected;
        /* Only two rows change when a detent lands. Avoid relaying out
         * and invalidating all four translated Home labels on each tick. */
        update_selection_row(previous);
        update_selection_row(selected);
    }
    queue_touched_home_selection();
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
    const lv_image_dsc_t *image = crazypod_custom_home_wallpaper();
    const lv_font_t *body_font = metadata_font != NULL
        ? metadata_font : &lv_font_montserrat_12;
    int i;

    memset(&desktop_host, 0, sizeof(desktop_host));
    if(host != NULL)
        desktop_host = *host;
    screen = lv_obj_create(NULL);
    crazypod_ui_widget_make_plain(screen);
    lv_obj_set_style_bg_color(screen, lv_color_hex(HOME_INK), 0);
    lv_obj_set_style_bg_grad_color(
        screen, lv_color_hex(CRAZYPOD_LM_HOME_BOTTOM), 0);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    wallpaper = lv_image_create(screen);
    if(image != NULL)
        lv_image_set_src(wallpaper, image);
    else
        lv_obj_add_flag(wallpaper, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(wallpaper, 0, 0);
    lv_obj_remove_flag(wallpaper, LV_OBJ_FLAG_CLICKABLE);
    /* Soft white scrim preserves the dark-text contrast over any user's
     * wallpaper. Leave the clean gradient visible when no art is set. */
    wallpaper_dim = crazypod_ui_widget_box(
        screen, 0, 0, LCD_WIDTH, LCD_HEIGHT, 0,
        HOME_GLASS, image != NULL
            ? CRAZYPOD_LM_HOME_WALLPAPER_SCRIM : LV_OPA_TRANSP);
    lv_obj_remove_flag(wallpaper_dim, LV_OBJ_FLAG_CLICKABLE);
    crazypod_status_bar_create(0, screen);
    crazypod_status_bar_set_palette(
        0, CRAZYPOD_LM_HOME_TEXT, CRAZYPOD_LM_HOME_GLASS);

    /* A new iPod: type-first identity, not a miniature iPhone launcher. */
    home_headline = crazypod_ui_widget_label(
        screen, "iPod", &lv_font_montserrat_24,
        CRAZYPOD_LM_HOME_TEXT, LV_OPA_COVER);
    lv_obj_set_pos(home_headline, HOME_ROW_X + 7, 32);
    lv_obj_remove_flag(home_headline, LV_OBJ_FLAG_CLICKABLE);

    /* Four legible destinations, with no app-carousel icon decoding. */
    for(i = 0; i < CRAZYPOD_HOME_DESTINATION_COUNT; ++i) {
        home_rows[i] = crazypod_ui_widget_box(
            screen, HOME_ROW_X,
            HOME_ROW_TOP + i * (HOME_ROW_HEIGHT + HOME_ROW_GAP),
            HOME_ROW_WIDTH, HOME_ROW_HEIGHT, 14,
            HOME_GLASS, LV_OPA_TRANSP);
        home_badges[i] = crazypod_ui_widget_box(
            home_rows[i], 7, 3, 19, 19, 6,
            home_entry_color(i), LV_OPA_COVER);
        lv_obj_remove_flag(
            home_badges[i], LV_OBJ_FLAG_CLICKABLE);
        home_symbols[i] = crazypod_ui_widget_label(
            home_badges[i], home_entry_symbol(i),
            &lv_font_montserrat_12, COLOR_WHITE, LV_OPA_COVER);
        lv_obj_center(home_symbols[i]);
        lv_obj_remove_flag(
            home_symbols[i], LV_OBJ_FLAG_CLICKABLE);
        home_labels[i] = crazypod_ui_widget_label(
            home_rows[i], home_entry_title(i),
            body_font, CRAZYPOD_LM_HOME_TEXT, LV_OPA_COVER);
        lv_obj_set_pos(home_labels[i], 35, 3);
        lv_obj_set_width(home_labels[i], HOME_ROW_WIDTH - 79);
        lv_label_set_long_mode(home_labels[i], LV_LABEL_LONG_MODE_DOTS);
        home_chevrons[i] = crazypod_ui_widget_label(
            home_rows[i], LV_SYMBOL_RIGHT, &lv_font_montserrat_12,
            CRAZYPOD_LM_HOME_TEXT, 150);
        lv_obj_set_pos(home_chevrons[i], HOME_ROW_WIDTH - 27, 6);
        lv_obj_remove_flag(
            home_chevrons[i], LV_OBJ_FLAG_CLICKABLE);
    }
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
    bool changed;

    (void)animated;
    selected = clamp_selected(selected);
    changed = selected != selected_app;
    wheel_tracking = false;
    wheel_detent_accumulator = 0;
    if(changed) {
        int previous = selected_app;
        selected_app = selected;
        update_selection_row(previous);
        update_selection_row(selected);
        queue_touched_home_selection();
    }
}

void crazypod_desktop_move_selection(int direction)
{
    int count = CRAZYPOD_HOME_DESTINATION_COUNT;
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
    if(wallpaper_dim != NULL)
        lv_obj_set_style_bg_opa(
            wallpaper_dim, custom != NULL
                ? CRAZYPOD_LM_HOME_WALLPAPER_SCRIM : LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_color(screen, lv_color_hex(HOME_INK), 0);
    crazypod_status_bar_set_palette(
        0, CRAZYPOD_LM_HOME_TEXT, CRAZYPOD_LM_HOME_GLASS);
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
