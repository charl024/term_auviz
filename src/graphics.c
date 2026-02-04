#include <notcurses/notcurses.h>
#include <stdlib.h>
#include <math.h>
#include <wchar.h>

#include "graphics.h"

#define MIN_EDGE 1
#define DB_MIN   (-80.0)
#define DB_MAX   (0.0)

#define RGB_R(c) (((c) >> 16) & 0xFF)
#define RGB_G(c) (((c) >>  8) & 0xFF)
#define RGB_B(c) (((c) >>  0) & 0xFF)

#define BG_COLOR  0x232136
#define BAR_COLOR 0xebbcba

static struct ncplane *root;
static struct ncplane *main_win;

static uint64_t bg_channels = NCCHANNELS_INITIALIZER(
    0, 0, 0,
    RGB_R(BG_COLOR),
    RGB_G(BG_COLOR),
    RGB_B(BG_COLOR)
);

static uint64_t bar_channels = NCCHANNELS_INITIALIZER(
    0, 0, 0,
    RGB_R(BAR_COLOR),
    RGB_G(BAR_COLOR),
    RGB_B(BAR_COLOR)
);

static void draw_bars(app_state_t *state, int num_bins)
{
    int cols = state->main_cols - 2;
    int rows = state->main_rows - 2;

    if (cols <= 0 || rows <= 0) {
        return;
    }

    int bins_per_bar = num_bins / cols;

    if (bins_per_bar < 1) {
        bins_per_bar = 1;
    }

    for (int x = 0; x < cols; x++) {
        double peak = DB_MIN;

        int start = x * bins_per_bar;
        int end   = start + bins_per_bar;
        if (end > num_bins) {
            end = num_bins;
        }

        for (int i = start; i < end; i++) {
            double v = state->buffer_data[i];
            if (v > peak)
                peak = v;
        }

        // clamp computed peak
        if (peak < DB_MIN) peak = DB_MIN;
        if (peak > DB_MAX) peak = DB_MAX;

        // norm step
        double norm = (peak - DB_MIN) / (DB_MAX - DB_MIN);

        int bar_height = (int)(norm * rows);

        ncplane_set_channels(main_win, bar_channels);
        for (int y = 0; y < bar_height; y++) {
            int draw_y = state->main_rows - 2 - y;
            int draw_x = x + 1;
    
            ncplane_putstr_yx(main_win, draw_y, draw_x, " ");
        }
    }
}

static void graphics_resize(struct notcurses *nc, app_state_t *state)
{
    if (main_win) {
        ncplane_destroy(main_win);
        main_win = NULL;
    }

    root = notcurses_stdplane(nc);
    ncplane_dim_yx(root, &state->term_rows, &state->term_cols);

    struct ncplane_options opts = {
        .y = 1,
        .x = 2,
        .rows = state->term_rows - 2,
        .cols = state->term_cols - 4,
    };

    if (opts.rows < 3 || opts.cols < 3) {
        return;
    }

    main_win = ncplane_create(root, &opts);
    ncplane_dim_yx(main_win, &state->main_rows, &state->main_cols);

    state->min_x = MIN_EDGE;
    state->min_y = MIN_EDGE;
    state->max_x = state->main_cols - MIN_EDGE;
    state->max_y = state->main_rows - MIN_EDGE;
}



void graphics_init(struct notcurses *nc, app_state_t *state) 
{
    root = notcurses_stdplane(nc);
    ncplane_dim_yx(root, &state->term_rows, &state->term_cols);

    state->last_term_rows = state->term_rows;
    state->last_term_cols = state->term_cols;


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

void graphics_draw(struct notcurses *nc, app_state_t *state, size_t num_bins) 
{
    int cur_rows, cur_cols;
    ncplane_dim_yx(notcurses_stdplane(nc), &cur_rows, &cur_cols);

    if (cur_rows != state->last_term_rows ||
        cur_cols != state->last_term_cols) {

        state->last_term_rows = cur_rows;
        state->last_term_cols = cur_cols;
        state->needs_resize = 1;
    }

    if (state->needs_resize) {
        graphics_resize(nc, state);
        state->needs_resize = 0;
    }
    

    if (!main_win || state->main_rows < 3 || state->main_cols < 3) {
        return;
    }


    ncplane_set_base(main_win, " ", 0, bg_channels);
    ncplane_erase(main_win);

    ncplane_double_box_sized(main_win, NCSTYLE_NONE, 0, state->main_rows, state->main_cols, 0);
    draw_bars(state, num_bins);
}

void graphics_shutdown(void)
{
    if (main_win) {
        ncplane_destroy(main_win);
        main_win = NULL;
    }
}

