#include <assert.h>

#include "../apps/crazypod/ui/shell/crazypod_home_model.h"

int main(void)
{
    /* Four product destinations are independent of legacy app IDs. */
    assert(CRAZYPOD_HOME_NOW_PLAYING == 0);
    assert(CRAZYPOD_HOME_MUSIC == 1);
    assert(CRAZYPOD_HOME_PODCASTS == 2);
    assert(CRAZYPOD_HOME_SETTINGS == 3);
    assert(CRAZYPOD_HOME_DESTINATION_COUNT == 4);
    return 0;
}
