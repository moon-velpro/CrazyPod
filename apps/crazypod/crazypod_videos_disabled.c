#include "config.h"

#ifdef IPOD_6G

#include "crazypod_videos.h"

/*
 * Music-only firmware has no video library or MPEG decoder.
 *
 * Keep the existing video interface linkable until the retired Media
 * routes and screens are deleted in a later, independently tested pass.
 * All operations are inert; no path is scanned, mutated, or opened.
 * This file intentionally depends on neither Rockbox media services nor
 * the MPEG player library.
 */
void crazypod_videos_init(void) {}
void crazypod_videos_refresh(void) {}
void crazypod_videos_ensure_catalog(void) {}
void crazypod_videos_suspend(void) {}
void crazypod_videos_resume(void) {}
void crazypod_videos_set_lock_suspended(bool suspended)
{
    (void)suspended;
}
void crazypod_videos_set_route_suspended(bool suspended)
{
    (void)suspended;
}
void crazypod_videos_invalidate_catalog(void) {}

int crazypod_video_count(void)
{
    return 0;
}
const char *crazypod_video_path(int index)
{
    (void)index;
    return NULL;
}
const char *crazypod_video_name(int index)
{
    (void)index;
    return NULL;
}
uint32_t crazypod_video_resume_seconds(int index)
{
    (void)index;
    return 0;
}
uint32_t crazypod_video_duration_seconds(int index)
{
    (void)index;
    return 0;
}
const lv_image_dsc_t *crazypod_video_poster(int index)
{
    (void)index;
    return NULL;
}
unsigned crazypod_video_generation(void)
{
    return 0;
}
bool crazypod_videos_busy(void)
{
    return false;
}
bool crazypod_video_delete(int index)
{
    (void)index;
    return false;
}
enum crazypod_video_result crazypod_video_play(int index)
{
    (void)index;
    return CRAZYPOD_VIDEO_UNSUPPORTED;
}
enum crazypod_video_result crazypod_video_last_result(void)
{
    return CRAZYPOD_VIDEO_UNSUPPORTED;
}
const char *crazypod_video_result_message(
    enum crazypod_video_result result)
{
    return result == CRAZYPOD_VIDEO_OK
        ? "" : "Video disabled in music-only firmware";
}

#endif
