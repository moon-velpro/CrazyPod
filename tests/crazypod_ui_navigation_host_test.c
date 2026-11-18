#include <assert.h>

#include "navigation/crazypod_ui_routes.h"

static void test_settings_value_route_restores_parent_selection(void)
{
    struct route_state *state;

    crazypod_ui_routes_reset(SETTINGS_ROUTE_MENU, -1, 3);
    assert(crazypod_ui_routes_push(SETTINGS_ROUTE_PLAYBACK, -1, 2));
    assert(crazypod_ui_routes_push(
        SETTINGS_ROUTE_VALUE_CHOICES, 17, 1));
    state = crazypod_ui_routes_current();
    assert(state != NULL);
    assert(state->route == SETTINGS_ROUTE_VALUE_CHOICES);
    assert(state->group == 17);
    assert(state->selected == 1);
    state->selected = 2;
    assert(crazypod_ui_routes_pop());
    state = crazypod_ui_routes_current();
    assert(state->route == SETTINGS_ROUTE_PLAYBACK);
    assert(state->selected == 2);
    assert(crazypod_ui_routes_pop());
    state = crazypod_ui_routes_current();
    assert(state->route == SETTINGS_ROUTE_MENU);
    assert(state->selected == 3);
    assert(!crazypod_ui_routes_pop());
}

static void test_nested_settings_actions_restore_item(void)
{
    struct route_state *state;

    crazypod_ui_routes_reset(SETTINGS_ROUTE_MENU, -1, 6);
    assert(crazypod_ui_routes_push(SETTINGS_ROUTE_MAIN_MENU, -1, 5));
    assert(crazypod_ui_routes_push(
        SETTINGS_ROUTE_MAIN_MENU_ACTIONS, 3, 0));
    state = crazypod_ui_routes_current();
    assert(state->group == 3);
    assert(crazypod_ui_routes_pop());
    state = crazypod_ui_routes_current();
    assert(state->route == SETTINGS_ROUTE_MAIN_MENU);
    assert(state->selected == 5);
}

static void test_full_route_stack_does_not_corrupt_parent(void)
{
    int depth;

    crazypod_ui_routes_reset(MUSIC_ROUTE_MENU, -1, 4);
    for(depth = 1; depth < 8; ++depth)
        assert(crazypod_ui_routes_push(MUSIC_ROUTE_SONGS, depth, depth));
    assert(!crazypod_ui_routes_push(
        SETTINGS_ROUTE_VALUE_CHOICES, 0, 0));
    assert(crazypod_ui_routes_depth() == 8);
    crazypod_ui_routes_truncate(1);
    assert(crazypod_ui_routes_current()->route == MUSIC_ROUTE_MENU);
    assert(crazypod_ui_routes_current()->selected == 4);
}

int main(void)
{
    test_settings_value_route_restores_parent_selection();
    test_nested_settings_actions_restore_item();
    test_full_route_stack_does_not_corrupt_parent();
    return 0;
}
