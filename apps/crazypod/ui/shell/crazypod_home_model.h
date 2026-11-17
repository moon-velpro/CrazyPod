#ifndef CRAZYPOD_HOME_MODEL_H
#define CRAZYPOD_HOME_MODEL_H

/* Home is a product navigation contract, not a persisted app-ID order.
 * Keep these destinations shared by the Shell and input dispatcher. */
enum crazypod_home_destination {
    CRAZYPOD_HOME_NOW_PLAYING = 0,
    CRAZYPOD_HOME_MUSIC,
    CRAZYPOD_HOME_PODCASTS,
    CRAZYPOD_HOME_SETTINGS,
    CRAZYPOD_HOME_DESTINATION_COUNT,
};

#endif
