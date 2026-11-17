#include "config.h"

#ifdef IPOD_6G

#include "crazypod_iphone_app_icons.h"
#include "crazypod_private_icon_assets.h"

/* The four previously uploaded reference PNGs belong to the local-only
 * private pack. Never embed copyrighted app icons as GitHub source data.
 *
 * Public and CI builds return NULL, allowing the original CrazyPod
 * rounded-square app icons to provide a safe, freely replaceable fallback. */
const lv_image_dsc_t *crazypod_iphone_app_icon(
    enum crazypod_iphone_app_icon_id icon)
{
    return crazypod_private_icon_app(icon);
}

#endif
