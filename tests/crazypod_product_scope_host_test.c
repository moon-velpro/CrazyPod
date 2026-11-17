#include <assert.h>
#include "../apps/crazypod/crazypod_product_scope.h"

int main(void)
{
    const enum crazypod_app_id retired[] = {
        CRAZYPOD_APP_MINI_APPS,
        CRAZYPOD_APP_PHOTOS,
        CRAZYPOD_APP_CUSTOMIZE,
        CRAZYPOD_APP_WORKOUTS,
        CRAZYPOD_APP_BOOKS,
        CRAZYPOD_APP_NOTES,
        CRAZYPOD_APP_EXTRAS,
        CRAZYPOD_APP_CLOCK,
        CRAZYPOD_APP_CONTACTS,
        CRAZYPOD_APP_CALENDAR,
        CRAZYPOD_APP_STOPWATCH,
        CRAZYPOD_APP_GAMEBOY,
    };
    unsigned i;

    assert(CRAZYPOD_APP_MUSIC == 1);
    assert(CRAZYPOD_APP_PODCASTS == 2);
    assert(CRAZYPOD_APP_GAMEBOY == 19);
    assert(CRAZYPOD_APP_SETTINGS == 14);
    assert(crazypod_product_is_application(CRAZYPOD_APP_MUSIC));
    assert(crazypod_product_is_application(CRAZYPOD_APP_PODCASTS));
    assert(crazypod_product_is_application(CRAZYPOD_APP_SETTINGS));
    assert(crazypod_product_launcher_allowed(CRAZYPOD_APP_SHUFFLE));
    assert(crazypod_product_launcher_allowed(CRAZYPOD_APP_LOCK));
    assert(!crazypod_product_is_application(CRAZYPOD_APP_SHUFFLE));
    assert(!crazypod_product_is_application(CRAZYPOD_APP_LOCK));
    assert(!crazypod_product_launcher_allowed(CRAZYPOD_APP_INVALID));

    for(i = 0; i < sizeof(retired) / sizeof(retired[0]); ++i) {
        assert(!crazypod_product_launcher_allowed(retired[i]));
        assert(!crazypod_product_is_application(retired[i]));
    }
    return 0;
}
