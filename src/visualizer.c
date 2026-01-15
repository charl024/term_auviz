#include "visualizer.h"

#define VSCALE 1.0

void update_bouncing(app_state_t *state, double dt) 
{
    double new_x = state->x_pos + (state->x_vel * dt * VSCALE);
    double new_y = state->y_pos + (state->y_vel * dt * VSCALE);

    if (new_x < (double)state->min_x) {
        new_x = state->min_x;
        state->x_vel = -state->x_vel;
    } else if (new_x >= (double)state->max_x) {
        new_x = (double)state->max_x;
        state->x_vel = -state->x_vel;
    }

    if (new_y < (double)state->min_y) {
        new_y = state->min_y;
        state->y_vel = -state->y_vel;
    } else if (new_y >= (double)state->max_y) {
        new_y = (double)state->max_y;
        state->y_vel = -state->y_vel;
    }

    state->x_pos = new_x;
    state->y_pos = new_y;
}

void update_visual_state(app_state_t* state, long elapsed_ns) 
{
    double dt = (double)elapsed_ns / 1e9;
    update_bouncing(state, dt);
}
