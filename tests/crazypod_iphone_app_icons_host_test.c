#include <assert.h>
#include <stdio.h>

#include "crazypod_iphone_app_icons.h"
#include "crazypod_private_icon_assets.h"

/* CI/public builds must not contain any private Apple PNG pixel payload. */
int main(void)
{
    int i;

    assert(crazypod_iphone_app_icon(
        (enum crazypod_iphone_app_icon_id)-1) == NULL);
    assert(crazypod_iphone_app_icon(
        CRAZYPOD_IPHONE_ICON_COUNT) == NULL);
    assert(crazypod_private_icon_menu(
        CRAZYPOD_MENU_ICON_NONE) == NULL);
    assert(crazypod_private_icon_menu(
        CRAZYPOD_MENU_ICON_COUNT) == NULL);
    for(i = 0; i < CRAZYPOD_IPHONE_ICON_COUNT; ++i)
        assert(crazypod_iphone_app_icon(
            (enum crazypod_iphone_app_icon_id)i) == NULL);
    for(i = CRAZYPOD_MENU_ICON_NONE + 1;
        i < CRAZYPOD_MENU_ICON_COUNT; ++i)
        assert(crazypod_private_icon_menu(
            (enum crazypod_menu_icon)i) == NULL);
    puts("CrazyPod public icon build excludes all private icon pixels");
    return 0;
}
