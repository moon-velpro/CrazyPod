#ifndef CRAZYPOD_HOME_ACTIONS_H
#define CRAZYPOD_HOME_ACTIONS_H

#include <stdbool.h>

#include "lvgl.h"
#include "../navigation/crazypod_ui_routes.h"

/* The Home quick actions are Shell routes, not modal UI state. */
bool crazypod_home_actions_is_route(enum crazypod_route route);
bool crazypod_home_actions_is_adjustment(enum crazypod_route route);
int crazypod_home_actions_item_count(enum crazypod_route route);
const char *crazypod_home_actions_item_title(int index);
const char *crazypod_home_actions_title(enum crazypod_route route);
bool crazypod_home_actions_adjust(
    enum crazypod_route route, int direction);
void crazypod_home_actions_render_adjustment(
    enum crazypod_route route, lv_obj_t *parent);

#endif
