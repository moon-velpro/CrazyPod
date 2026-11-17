#include "config.h"

#ifdef IPOD_6G

#include "audio.h"
#include "backlight.h"
#include "kernel.h"
#include "lvgl.h"

#include "../../crazypod_artwork.h"
#include "../../crazypod_coverflow.h"
#include "../../crazypod_frameclock.h"
#include "../../crazypod_music.h"
#include "../../crazypod_state.h"
#include "../features/music/crazypod_music_feature.h"
#include "../features/now_playing/crazypod_now_playing_feature.h"
#include "../shell/crazypod_desktop.h"
#include "../shell/crazypod_lock_screen.h"
#include "../shell/crazypod_now_capsule.h"
#include "../shell/crazypod_system_prompts.h"
#include "crazypod_runtime_services.h"
#include "crazypod_screen_off_policy.h"

/*
 * Music-only scheduler.
 *
 * The old product serviced MiniApp installers, alarms, CPK themes,
 * photo/video catalogs, notes, books, and organizer routes on every UI
 * iteration even though none of them were reachable from Home.
 * Preserve the audio, cover cache, music scan, UI motion, state persistence,
 * and lock/screen-off checkpoints; do not poll retired application services.
 */
static bool screen_off_idle;

#define CRAZYPOD_STATIC_WAIT_TICKS \
    (HZ > 0 ? HZ : 1)
#define CRAZYPOD_CLOCK_WAIT_TICKS \
    ((HZ / 4) > 0 ? (HZ / 4) : 1)
#define CRAZYPOD_PLAYBACK_WAIT_TICKS \
    ((HZ / 10) > 0 ? (HZ / 10) : 1)
#define CRAZYPOD_MEDIA_WAIT_TICKS \
    ((HZ / 20) > 0 ? (HZ / 20) : 1)
#define CRAZYPOD_FRAME_WAIT_TICKS \
    ((HZ / CRAZYPOD_TARGET_FPS) > 0 \
        ? (HZ / CRAZYPOD_TARGET_FPS) : 1)

void crazypod_runtime_services_configure(
    void (*render)(bool transition))
{
    /* Legacy scene rendering callback has no background app to service. */
    (void)render;
}

void crazypod_runtime_services_start(void)
{
    screen_off_idle = false;
    crazypod_now_playing_theme_prepare();
    crazypod_now_capsule_refresh_material();
}

static int runtime_wait_ticks(void)
{
    int status;

    if(crazypod_lock_screen_is_locked()) {
        if(crazypod_lock_screen_motion_active())
            return CRAZYPOD_FRAME_WAIT_TICKS;
        if(is_backlight_on(true)) {
            if(crazypod_artwork_busy())
                return CRAZYPOD_MEDIA_WAIT_TICKS;
            if((audio_status() & AUDIO_STATUS_PLAY) != 0)
                return CRAZYPOD_PLAYBACK_WAIT_TICKS;
        }
        return CRAZYPOD_STATIC_WAIT_TICKS;
    }

    if(lv_anim_count_running() ||
       crazypod_desktop_motion_active() ||
       crazypod_coverflow_motion_active())
        return CRAZYPOD_FRAME_WAIT_TICKS;
    if(crazypod_artwork_busy())
        return CRAZYPOD_MEDIA_WAIT_TICKS;
    if(crazypod_music_is_scanning() ||
       crazypod_music_catalog_validation() ==
           CRAZYPOD_MUSIC_VALIDATION_RUNNING)
        return CRAZYPOD_CLOCK_WAIT_TICKS;

    status = audio_status();
    if((status & AUDIO_STATUS_PLAY) != 0 &&
       (status & AUDIO_STATUS_PAUSE) == 0)
        return CRAZYPOD_PLAYBACK_WAIT_TICKS;

    return CRAZYPOD_STATIC_WAIT_TICKS;
}

void crazypod_runtime_services_tick(
    long now, bool frame_due, bool locked)
{
    (void)frame_due;

    /* Keep scanning and media cache suspensions aligned with Hold/lock. */
    crazypod_music_set_scan_suspended(locked);
    crazypod_artwork_set_lock_suspended(
        locked && !is_backlight_on(true));
    crazypod_music_library_service(
        now, locked || crazypod_system_prompts_storage_active());
}

int crazypod_runtime_services_prepare_wait(
    long now, bool *screen_off)
{
    bool active = crazypod_lock_screen_is_locked() &&
        !is_backlight_on(true);

    *screen_off = active;
    if(!active) {
        screen_off_idle = false;
        return runtime_wait_ticks();
    }
    if(!screen_off_idle) {
        crazypod_runtime_services_tick(now, false, true);
        crazypod_state_save(false);
        screen_off_idle = true;
    }

    /* Product alarms came only from the removed MiniApp runtime.
     * The persisted music state remains the source of any earlier wakeup. */
    return crazypod_screen_off_shorter_wait(
        CRAZYPOD_STATIC_WAIT_TICKS, crazypod_state_wait_ticks());
}

bool crazypod_runtime_services_screen_off_tick(void)
{
    if(!crazypod_lock_screen_is_locked() ||
       is_backlight_on(true)) {
        screen_off_idle = false;
        return false;
    }
    crazypod_state_tick();
    return true;
}

#endif
