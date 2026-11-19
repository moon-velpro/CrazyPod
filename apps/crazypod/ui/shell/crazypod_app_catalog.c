#include "config.h"

#include "../../crazypod_l10n.h"

#ifdef IPOD_6G

#include "lvgl.h"

#include "crazypod_app_catalog.h"

static const struct crazypod_app_descriptor catalog[CRAZYPOD_APP_COUNT] = {
    { CRAZYPOD_APP_MUSIC, CP_TR("Music"), LV_SYMBOL_AUDIO,
      CRAZYPOD_MENU_ICON_MUSIC, 0xFF2E54 },
    { CRAZYPOD_APP_PODCASTS, CP_TR("Podcasts"), LV_SYMBOL_VOLUME_MAX,
      CRAZYPOD_MENU_ICON_PODCAST, 0xA95BDE },
    { CRAZYPOD_APP_SHUFFLE, CP_TR("Shuffle"), LV_SYMBOL_SHUFFLE,
      CRAZYPOD_MENU_ICON_SHUFFLE, 0xFF375F },
    { CRAZYPOD_APP_LOCK, CP_TR("Lock"), LV_SYMBOL_EYE_CLOSE,
      CRAZYPOD_MENU_ICON_LOCK, 0x59606B },
    { CRAZYPOD_APP_SETTINGS, CP_TR("Settings"), LV_SYMBOL_SETTINGS,
      CRAZYPOD_MENU_ICON_SETTINGS, 0x8E8E93 },
};

const struct crazypod_app_descriptor *crazypod_app_catalog_at(int index)
{
    return index >= 0 && index < CRAZYPOD_APP_COUNT
        ? &catalog[index] : NULL;
}

int crazypod_app_catalog_index(enum crazypod_app_id id)
{
    int i;

    for(i = 0; i < CRAZYPOD_APP_COUNT; ++i) {
        if(catalog[i].id == id)
            return i;
    }
    return -1;
}

const struct crazypod_app_descriptor *crazypod_app_catalog_find(
    enum crazypod_app_id id)
{
    return crazypod_app_catalog_at(crazypod_app_catalog_index(id));
}

#endif
