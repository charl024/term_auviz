#include <notcurses/notcurses.h>
#include <stdlib.h>
#include <math.h>
#include <wchar.h>

#include "graphics.h"

#define MAX_HISTORY_BARS 512
#define MIN_EDGE 1
#define DB_MIN   (-80.0)
#define DB_MAX   (0.0)

#define RGB_R(c) (((c) >> 16) & 0xFF)
#define RGB_G(c) (((c) >>  8) & 0xFF)
#define RGB_B(c) (((c) >>  0) & 0xFF)

#define BG_COLOR    0x232136
#define MW_BG_COLOR 0x191724
#define BAR_COLOR   0xebbcba

// bar visuals
#define ATTACK     0.7   // [0.0–1.0] How fast bars rise toward new input (higher -> faster)
#define DECAY      0.9   // [0.0–1.0] How fast bars fall when input drops (higher -> slower fall)
#define PEAK_MAG   0.5   // [0.0–1.0] Threshold/boost for emphasizing peaks (higher -> stronger peak emphasis)
#define SMOOTH_ALPHA 0.2 // [0.0–1.0] Temporal smoothing factor (higher -> less smoothing, more reactive)

static struct ncplane *root;
static struct ncplane *main_win;
static double prev_heights[MAX_HISTORY_BARS] = {0};


static uint64_t bg_channels = NCCHANNELS_INITIALIZER(
    0, 0, 0,
    RGB_R(BG_COLOR),
    RGB_G(BG_COLOR),
    RGB_B(BG_COLOR)
);

static uint64_t main_win_bg_channels = NCCHANNELS_INITIALIZER(
    0, 0, 0,
    RGB_R(MW_BG_COLOR),
    RGB_G(MW_BG_COLOR),
    RGB_B(MW_BG_COLOR)
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

    // fprintf(stderr, "%d\n", state->main_cols);

    if (cols <= 0 || rows <= 0) {
        return;
    }

    int bins_per_bar = num_bins / cols;

    if (bins_per_bar < 1) {
        bins_per_bar = 1;
    }

    for (int x = 0; x < cols; x++) {
        if (x >= MAX_HISTORY_BARS) {
            break;
        }

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
        double norm = pow((peak - DB_MIN) / (DB_MAX - DB_MIN), PEAK_MAG);

        double target_height = norm * rows;

        prev_heights[x] = (1.0 - SMOOTH_ALPHA) * prev_heights[x] + SMOOTH_ALPHA * target_height;

        // if (target_height > prev_heights[x]) {
        //     prev_heights[x] = prev_heights[x] * (1.0 - ATTACK) + target_height * ATTACK;
        // } else {
        //     prev_heights[x] *= DECAY;
        // }

        // int bar_height = (int)prev_heights[x];

        // ncplane_set_channels(main_win, bar_channels);
        // for (int y = 0; y < bar_height; y++) {
        //     int draw_y = state->main_rows - 2 - y;
        //     int draw_x = x + 1;
    
        //     ncplane_putstr_yx(main_win, draw_y, draw_x, " ");
        // }
    }

    double temp[MAX_HISTORY_BARS];

    for (int x = 0; x < cols; x++) {
        temp[x] = prev_heights[x];
    }

    // box filter
    // for (int x = 1; x < cols - 1; x++) {
    //     prev_heights[x] = (temp[x - 1] + temp[x] + temp[x + 1]) / 3.0;
    // }

    // double weights[5] = {0.061, 0.244, 0.387, 0.244, 0.061};

    // for (int x = 2; x < cols - 2; x++) {
    //     int val = weights[0] * temp[x - 2] + weights[1] * temp[x - 1] + weights[2] * temp[x];
    //     int val2 = weights[3] * temp[x + 1] + weights[4] * temp[x + 2];
    //     prev_heights[x] = val + val2;
    // }

    double weights[5] = {0.244, 0.387, 0.244};

    for (int x = 1; x < cols - 1; x++) {
        int val = weights[0] * temp[x - 1] + weights[1] * temp[x] + weights[2] * temp[x + 1];
        prev_heights[x] = val;
    }

    ncplane_set_channels(main_win, bar_channels);

    for (int x = 0; x < cols && x < MAX_HISTORY_BARS; x++) {

        int bar_height = (int)prev_heights[x];

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


    ncplane_set_base(root, " ", 0, bg_channels);

    ncplane_set_base(main_win, " ", 0, main_win_bg_channels);
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

