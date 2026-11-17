#ifndef CRAZYPOD_HOME_MODEL_H
#define CRAZYPOD_HOME_MODEL_H

#include "../../crazypod_product_geometry.h"

/* Home is a product navigation contract, not a persisted app-ID order.
 * Keep these destinations shared by the Shell and input dispatcher. */
enum crazypod_home_destination {
    CRAZYPOD_HOME_NOW_PLAYING = 0,
    CRAZYPOD_HOME_MUSIC,
    CRAZYPOD_HOME_PODCASTS,
    CRAZYPOD_HOME_SETTINGS,
    CRAZYPOD_HOME_DESTINATION_COUNT,
};


/* A fixed viewport clips the vertical reel. Shift only its track
 * when the Click Wheel changes selection: all four app objects stay alive,
 * avoiding icon decode, label allocation and layout churn per detent. */
static inline int crazypod_home_reel_offset_y(int selected)
{
    return CRAZYPOD_PRODUCT_HOME_FOCUSED_Y -
           CRAZYPOD_PRODUCT_HOME_VIEWPORT_Y -
           CRAZYPOD_PRODUCT_HOME_TRACK_PADDING -
           selected * CRAZYPOD_PRODUCT_HOME_CARD_STEP;
}

/* Accelerated wheel events may skip multiple rows. Always stop at the
 * nearest edge instead of dropping an overshooting event entirely. */
static inline int crazypod_home_selection_after_wheel(
    int selected, int steps)
{
    int target = selected + steps;

    if(target < CRAZYPOD_HOME_NOW_PLAYING)
        return CRAZYPOD_HOME_NOW_PLAYING;
    if(target >= CRAZYPOD_HOME_DESTINATION_COUNT)
        return CRAZYPOD_HOME_DESTINATION_COUNT - 1;
    return target;
}

#endif
