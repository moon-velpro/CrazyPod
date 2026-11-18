#include "config.h"

#ifdef IPOD_6G

#include <stddef.h>

#include "crazypod_private_icon_assets.h"

#ifdef CRAZYPOD_PRIVATE_ICONS
/* Fake/controlled data for the C99 host fixture test only; real private
 * builds include an ignored locally generated source, never a Git blob. */
#ifdef CRAZYPOD_PRIVATE_ICON_TEST_FIXTURE
#include <crazypod_private_icon_fixture.inc>
#else
#include "crazypod_private_icon_assets.inc"
#endif
#endif

const lv_image_dsc_t *crazypod_private_icon_app(
    enum crazypod_iphone_app_icon_id icon)
{
    /* This enum's first enumerator is zero; negative casts are unsigned. */
    if(icon >= CRAZYPOD_IPHONE_ICON_COUNT)
        return NULL;
#ifdef CRAZYPOD_PRIVATE_ICONS
    return crazypod_local_app_icons[icon];
#else
    return NULL;
#endif
}

const lv_image_dsc_t *crazypod_private_icon_menu(
    enum crazypod_menu_icon icon)
{
    if(icon <= CRAZYPOD_MENU_ICON_NONE ||
       icon >= CRAZYPOD_MENU_ICON_COUNT)
        return NULL;
#ifdef CRAZYPOD_PRIVATE_ICONS
    return crazypod_local_menu_icons[icon];
#else
    return NULL;
#endif
}

#endif
