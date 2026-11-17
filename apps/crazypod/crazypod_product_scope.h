#ifndef CRAZYPOD_PRODUCT_SCOPE_H
#define CRAZYPOD_PRODUCT_SCOPE_H

/*
 * Product scope, independent of legacy on-disk application order.
 * Do not renumber enum crazypod_app_id: old settings/backup images contain
 * those numeric IDs. This is a new product availability decision only.
 */
#include "crazypod_apps.h"

static inline bool crazypod_product_is_application(
    enum crazypod_app_id id)
{
    return id == CRAZYPOD_APP_MUSIC ||
           id == CRAZYPOD_APP_PODCASTS ||
           id == CRAZYPOD_APP_SETTINGS;
}

/* Shuffle and Hold/Lock are transport/device actions, not applications. */
static inline bool crazypod_product_launcher_allowed(
    enum crazypod_app_id id)
{
    return crazypod_product_is_application(id) ||
           id == CRAZYPOD_APP_SHUFFLE ||
           id == CRAZYPOD_APP_LOCK;
}

#endif
