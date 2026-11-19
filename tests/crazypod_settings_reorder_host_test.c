#include <assert.h>

#include "crazypod_apps.h"
#include "features/settings/crazypod_settings_catalog.h"
#include "features/settings/crazypod_settings_controller.h"

bool crazypod_settings_catalog_handles(enum crazypod_route route)
{
    return route >= SETTINGS_ROUTE_MENU &&
        route <= SETTINGS_ROUTE_MAIN_MENU_ACTIONS;
}

int crazypod_settings_catalog_count(enum crazypod_route route)
{
    (void)route;
    return 0;
}

int crazypod_settings_catalog_item(
    enum crazypod_route route, int index)
{
    (void)route;
    (void)index;
    return -1;
}

static void test_main_menu_opens_actions_overlay(void)
{
    struct route_state state = {
        .route = SETTINGS_ROUTE_MAIN_MENU,
        .selected = 2,
        .group = -1,
    };
    struct crazypod_settings_command command;

    crazypod_apps_reset();
    command = crazypod_settings_activate(&state);
    assert(command.kind ==
        CRAZYPOD_SETTINGS_COMMAND_SHOW_MAIN_MENU_ACTIONS);
    assert(command.app_id == CRAZYPOD_APP_SHUFFLE);
}

static void test_direct_reorder_transaction(void)
{
    enum crazypod_app_id id = CRAZYPOD_APP_SHUFFLE;
    int original;

    crazypod_apps_reset();
    original = crazypod_apps_order_index(id);
    crazypod_settings_begin_main_menu_reorder(id);
    assert(crazypod_settings_main_menu_reordering());
    assert(crazypod_settings_main_menu_reorder_id() == id);
    assert(crazypod_settings_move_main_menu_item(1));
    assert(crazypod_apps_order_index(id) == original + 1);
    assert(crazypod_settings_move_main_menu_item(-1));
    assert(crazypod_apps_order_index(id) == original);
    assert(crazypod_settings_finish_main_menu_reorder() == id);
    assert(!crazypod_settings_main_menu_reordering());
    assert(!crazypod_settings_move_main_menu_item(1));
}

static void test_reorder_respects_bounds(void)
{
    crazypod_apps_reset();
    crazypod_settings_begin_main_menu_reorder(
        CRAZYPOD_APP_MUSIC);
    assert(!crazypod_settings_move_main_menu_item(-1));
    assert(crazypod_apps_order_index(
        CRAZYPOD_APP_MUSIC) == 0);
    assert(crazypod_settings_finish_main_menu_reorder() ==
        CRAZYPOD_APP_MUSIC);
}

static void test_retired_app_ignored_after_legacy_restore(void)
{
    uint8_t old_order[CRAZYPOD_APP_DISK_COUNT] = {
        CRAZYPOD_APP_MUSIC, CRAZYPOD_APP_PODCASTS,
        CRAZYPOD_APP_MINI_APPS, CRAZYPOD_APP_SHUFFLE,
        CRAZYPOD_APP_LOCK, CRAZYPOD_APP_PHOTOS,
        CRAZYPOD_APP_CUSTOMIZE, CRAZYPOD_APP_WORKOUTS,
        CRAZYPOD_APP_BOOKS, CRAZYPOD_APP_NOTES,
        CRAZYPOD_APP_CLOCK, CRAZYPOD_APP_CONTACTS,
        CRAZYPOD_APP_CALENDAR, CRAZYPOD_APP_STOPWATCH,
        CRAZYPOD_APP_EXTRAS, CRAZYPOD_APP_SETTINGS,
        CRAZYPOD_APP_RETIRED_19,
    };
    uint8_t restored_order[CRAZYPOD_APP_COUNT];
    uint32_t enabled_mask;
    uint32_t restored_mask;
    int i;

    crazypod_apps_reset();
    assert(crazypod_apps_count() == CRAZYPOD_APP_COUNT);
    assert(CRAZYPOD_APP_COUNT == 5);
    assert(CRAZYPOD_APP_LEGACY_COUNT == 16);
    enabled_mask = UINT32_MAX;

    crazypod_apps_restore(old_order, sizeof(old_order), enabled_mask);
    assert(!crazypod_apps_is_known(CRAZYPOD_APP_RETIRED_19));
    assert(!crazypod_apps_is_enabled(CRAZYPOD_APP_RETIRED_19));
    assert(crazypod_apps_visible_index(CRAZYPOD_APP_RETIRED_19) < 0);
    crazypod_apps_export(
        restored_order, sizeof(restored_order), &restored_mask);
    assert((restored_mask &
            ((uint32_t)1u << CRAZYPOD_APP_RETIRED_19)) == 0);
    for(i = 0; i < crazypod_apps_count(); ++i) {
        enum crazypod_app_id id = crazypod_apps_ordered_id(i);

        assert(crazypod_apps_is_known(id));
        assert(id != CRAZYPOD_APP_RETIRED_19);
    }
    assert(crazypod_apps_count() == 5);
    assert(crazypod_apps_visible_count() == 5);
    assert(!crazypod_apps_is_known(CRAZYPOD_APP_MINI_APPS));
    assert(!crazypod_apps_is_known(CRAZYPOD_APP_BOOKS));
    assert(!crazypod_apps_is_known(CRAZYPOD_APP_PHOTOS));
    assert(!crazypod_apps_is_known(CRAZYPOD_APP_CUSTOMIZE));
    assert(!crazypod_apps_is_known(CRAZYPOD_APP_EXTRAS));
}

int main(void)
{
    test_main_menu_opens_actions_overlay();
    test_direct_reorder_transaction();
    test_reorder_respects_bounds();
    test_retired_app_ignored_after_legacy_restore();
    return 0;
}
