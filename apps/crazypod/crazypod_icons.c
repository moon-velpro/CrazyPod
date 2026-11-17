#include "config.h"

#ifdef IPOD_6G

#include <stddef.h>

#include "crazypod_icons.h"

/* The 160x160 BMP app carousel was retired with the old horizontal Home.
 * Keep its public functions as link-compatible no-ops while old settings
 * and cross-module clients are removed in independent refactor slices.
 *
 * The active Home uses small RGB565A8 app images prepared at build time,
 * and must not allocate 17 * 160 * 160 * 4 bytes or scan theme BMP files
 * during startup. Existing files on the user's drive remain untouched. */
void crazypod_icons_init(void)
{
}

void crazypod_icons_load_theme(int theme)
{
    (void)theme;
}

const struct crazypod_icon *crazypod_icon_get(int index)
{
    (void)index;
    return NULL;
}

#endif
