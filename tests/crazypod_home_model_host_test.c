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
    /* Fast wheel flicks clamp to the final entry instead of doing nothing. */
    assert(crazypod_home_selection_after_wheel(0, 1) == 1);
    assert(crazypod_home_selection_after_wheel(0, 3) == 3);
    assert(crazypod_home_selection_after_wheel(1, 3) == 3);
    assert(crazypod_home_selection_after_wheel(2, 3) == 3);
    assert(crazypod_home_selection_after_wheel(3, -3) == 0);
    assert(crazypod_home_selection_after_wheel(1, -3) == 0);
    assert(crazypod_home_selection_after_wheel(2, 0) == 2);
    assert(crazypod_home_selection_after_wheel(3, 1) == 3);
    return 0;
}
