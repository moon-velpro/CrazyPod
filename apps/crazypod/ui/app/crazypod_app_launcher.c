#include "config.h"

#ifdef IPOD_6G

#include "kernel.h"

#include "../../crazypod_music.h"
#include "../features/music/crazypod_music_feature.h"
#include "../../crazypod_state.h"
#include "../navigation/crazypod_route_registry.h"
#include "../navigation/crazypod_ui_routes.h"
#include "../shell/crazypod_shell.h"
#include "crazypod_app_launcher.h"
#include "crazypod_scene_transition.h"

static struct crazypod_app_launcher_host host;
static bool shuffle_pending;

static void open_music(void)
{
    (void)crazypod_music_validate_catalog_async();
    crazypod_shell_open_product();
    host.boost(true);
    crazypod_ui_routes_reset(MUSIC_ROUTE_MENU, -1, 0);
    if(!crazypod_music_library_loaded()) {
        host.begin_music_scan();
        return;
    }
    host.render(true);
}

void crazypod_app_launcher_open_now_playing(void)
{
    shuffle_pending = false;
    (void)crazypod_music_validate_catalog_async();
    crazypod_shell_open_product();
    host.boost(true);
    /* Home is the parent of this shortcut.  Leave Now Playing as the route
     * root so Menu closes the product layer instead of revealing Music. */
    crazypod_ui_routes_clear();
    host.request_now_playing();
}

static void open_shuffle_screen(void)
{
    crazypod_shell_open_product();
    host.boost(true);
    /* Shuffle is launched from Home, so Now Playing must be the root route.
     * Otherwise Menu reveals the transient Music loading route. */
    crazypod_ui_routes_clear();
    host.request_now_playing();
}

static void start_shuffle(void)
{
    if(!crazypod_music_shuffle_all((unsigned int)current_tick))
        return;
    crazypod_state_forget_resume();
    crazypod_state_mark_dirty();
}

static void request_shuffle(void)
{
    shuffle_pending = true;
    open_shuffle_screen();
    (void)crazypod_music_validate_catalog_async();
    if(!crazypod_music_library_loaded())
        host.begin_music_scan();
}

static void open_route(enum crazypod_route route)
{
    crazypod_shell_open_product();
    host.boost(true);
    crazypod_ui_routes_reset(route, -1, 0);
    host.render(true);
}

void crazypod_app_launcher_configure(
    const struct crazypod_app_launcher_host *new_host)
{
    if(new_host != NULL)
        host = *new_host;
}

/* A single route policy in navigation/crazypod_ui_routes.h owns the
 * product allowlist. Do not duplicate the enum range here. */
void crazypod_app_launcher_open_root(enum crazypod_route route)
{
    if(crazypod_ui_route_supported(route) &&
       crazypod_route_registry_get(route) != NULL)
        open_route(route);
}

/* Compatibility entry point until the retired books service is unlinked. */
void crazypod_app_launcher_open_books(void)
{
    /* Books are not part of the music-only product. */
}

void crazypod_app_launcher_open(enum crazypod_app_id id)
{
    if(!crazypod_apps_is_known(id))
        return;
    if(id != CRAZYPOD_APP_SHUFFLE)
        shuffle_pending = false;
    switch(id) {
    case CRAZYPOD_APP_MUSIC:
        shuffle_pending = false;
        open_music();
        break;
    case CRAZYPOD_APP_SHUFFLE:
        request_shuffle();
        break;
    case CRAZYPOD_APP_LOCK:
        host.show_lock(true);
        break;
    case CRAZYPOD_APP_PODCASTS:
        (void)crazypod_music_validate_catalog_async();
        open_route(PODCASTS_ROUTE_MENU);
        if(!crazypod_music_library_loaded())
            host.begin_music_scan();
        break;
    case CRAZYPOD_APP_SETTINGS:
        open_route(SETTINGS_ROUTE_MENU);
        break;
    default:
        break;
    }
}

void crazypod_app_launcher_process_pending(void)
{
    enum crazypod_music_catalog_validation validation;

    if(!shuffle_pending || crazypod_scene_transition_active() ||
       crazypod_music_is_scanning() ||
       !crazypod_music_library_loaded())
        return;
    validation = crazypod_music_catalog_validation();
    if(validation != CRAZYPOD_MUSIC_VALIDATION_CURRENT &&
       validation != CRAZYPOD_MUSIC_VALIDATION_FAILED)
        return;

    shuffle_pending = false;
    if(crazypod_music_track_count() > 0)
        start_shuffle();
}

void crazypod_app_launcher_cancel_pending(void)
{
    shuffle_pending = false;
}

#ifdef SIMULATOR
void crazypod_app_launcher_simulate_shuffle_ready(void)
{
    shuffle_pending = false;
    open_shuffle_screen();
}
#endif

#endif
