#include "config.h"

#ifdef IPOD_6G

#include "crazypod_private_icon_assets.h"

#ifdef CRAZYPOD_PRIVATE_ICONS
/* Generated only by the local, explicitly opted-in private build.
 * This file is excluded from Git and is never part of public CI. */
#include "crazypod_private_icon_assets.inc"
#endif

const lv_image_dsc_t *crazypod_private_icon_app(
    enum crazypod_iphone_app_icon_id icon)
{
    if(icon < CRAZYPOD_IPHONE_ICON_VOICE_MEMOS ||
       icon >= CRAZYPOD_IPHONE_ICON_COUNT)
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
