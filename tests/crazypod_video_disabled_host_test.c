#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "../apps/crazypod/crazypod_videos.h"

int main(void)
{
    /* Music-only build cannot scan, decode, display, or delete videos. */
    crazypod_videos_init();
    crazypod_videos_refresh();
    crazypod_videos_ensure_catalog();
    crazypod_videos_suspend();
    crazypod_videos_resume();
    crazypod_videos_set_lock_suspended(true);
    crazypod_videos_set_route_suspended(false);
    crazypod_videos_invalidate_catalog();

    assert(crazypod_video_count() == 0);
    assert(crazypod_video_path(0) == NULL);
    assert(crazypod_video_name(0) == NULL);
    assert(crazypod_video_resume_seconds(0) == 0);
    assert(crazypod_video_duration_seconds(0) == 0);
    assert(crazypod_video_poster(0) == NULL);
    assert(crazypod_video_generation() == 0);
    assert(!crazypod_videos_busy());
    assert(!crazypod_video_delete(0));
    assert(crazypod_video_play(0) == CRAZYPOD_VIDEO_UNSUPPORTED);
    assert(crazypod_video_last_result() == CRAZYPOD_VIDEO_UNSUPPORTED);
    assert(strcmp(crazypod_video_result_message(CRAZYPOD_VIDEO_OK), "") == 0);
    assert(crazypod_video_result_message(
        CRAZYPOD_VIDEO_UNSUPPORTED)[0] != '\0');
    return 0;
}
