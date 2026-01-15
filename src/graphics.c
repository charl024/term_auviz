#include <notcurses/notcurses.h>
#include "graphics.h"

static struct ncplane *root;
static struct ncplane *main_win;

#define MIN_EDGE 1

void graphics_init(struct notcurses *nc, app_state_t *state) 
{
    root = notcurses_stdplane(nc);
    ncplane_dim_yx(root, &state->term_rows, &state->term_cols);

    struct ncplane_options opts = {
        .y = 1,
        .x = 2,
        .rows = state->term_rows - 2,
        .cols = state->term_cols - 4,
    };

    main_win = ncplane_create(root, &opts);
    ncplane_dim_yx(main_win, &state->main_rows, &state->main_cols);

    state->min_x = MIN_EDGE;
    state->min_y = MIN_EDGE;
    state->max_x = state->main_cols - MIN_EDGE;
    state->max_y = state->main_rows - MIN_EDGE;
}

void graphics_draw(struct notcurses *nc, app_state_t *state) 
{
    (void)nc;

    ncplane_erase(main_win);

    ncplane_double_box_sized(main_win, NCSTYLE_NONE, 0, state->main_rows, state->main_cols, 0);
    ncplane_putstr_yx(main_win, (int)state->y_pos, (int)state->x_pos, "O");
    ncplane_putstr_aligned(main_win, 1, NCALIGN_CENTER, "wassup");
}

void graphics_shutdown(void)
{
    if (main_win) {
        ncplane_destroy(main_win);
        main_win = NULL;
    }
}
