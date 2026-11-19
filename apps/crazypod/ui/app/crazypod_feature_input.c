#include "config.h"

#ifdef IPOD_6G

#include "button.h"

#include "../features/music/crazypod_music_feature.h"
#include "../features/now_playing/crazypod_now_playing_feature.h"
#include "../features/settings/crazypod_settings_feature.h"
#include "../navigation/crazypod_input_event.h"
#include "crazypod_feature_input.h"
#include "crazypod_route_actions.h"

/* Only product-level feature input is bound. Legacy feature IDs remain in
 * the enum for serialized compatibility but receive no input handler. */
static struct crazypod_feature_input_host host;

static void activate(void)
{
    crazypod_route_actions_activate(host.now());
}

static void move(int direction)
{
    crazypod_route_actions_move(direction, host.now());
}

static bool handle_now_playing_raw(
    const struct route_state *state,
    const struct crazypod_input_event *event,
    void *context)
{
    (void)state;
    (void)context;
    if(event->base == BUTTON_MENU && event->repeated) {
        crazypod_route_actions_pop();
        return true;
    }
    return false;
}

static bool handle_music_pressed(
    const struct route_state *state,
    const struct crazypod_input_event *event,
    void *context)
{
    const struct crazypod_feature_input_context input = {
        .now = host.now(),
        .activate = activate,
        .move = move,
        .render = host.render,
        .push = crazypod_route_actions_push,
        .pop = crazypod_route_actions_pop,
    };

    (void)context;
    return crazypod_music_feature_handle_input(
        state, event, &input);
}

static bool handle_settings_pressed(
    const struct route_state *state,
    const struct crazypod_input_event *event,
    void *context)
{
    const struct crazypod_feature_input_context input = {
        .now = host.now(),
        .render = host.render,
        .move = move,
        .activate = activate,
        .pop = crazypod_route_actions_pop,
    };

    (void)context;
    return crazypod_settings_feature_handle_input(
        state, event, &input);
}

static const struct crazypod_feature_bindings bindings = {
    .raw = {
        [CRAZYPOD_FEATURE_NOW_PLAYING] = handle_now_playing_raw,
    },
    .pressed = {
        [CRAZYPOD_FEATURE_MUSIC] = handle_music_pressed,
        [CRAZYPOD_FEATURE_SETTINGS] = handle_settings_pressed,
    },
};

void crazypod_feature_input_configure(
    const struct crazypod_feature_input_host *new_host)
{
    if(new_host != NULL)
        host = *new_host;
}

const struct crazypod_feature_bindings *
crazypod_feature_input_bindings(void)
{
    return &bindings;
}

#endif
