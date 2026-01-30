#include <notcurses/notcurses.h>
#include <stdlib.h>
#include <math.h>
#include <wchar.h>

#include "graphics.h"

#define MIN_EDGE 1
#define MAX_BARS 128
#define BAR_HEIGHT_SCALE 2

#define MAX_HISTORY_BARS 512

#define SILENCE_THRESHOLD (-5.5)

#define SMOOTHING_ALPHA   0.1
#define DECAY_FACTOR      0.975
#define SILENCE_DECAY     0.9

#define DB_MIN (-80.0)
#define DB_MAX (0.0)

#define BLOCK_LEVELS      8

#define BG_BAR_COLOR      0xffffff
#define BG_CLEAR_COLOR    0x000000

#define BORDER_PADDING    2

static struct ncplane *root;
static struct ncplane *main_win;

static double prev_heights[MAX_HISTORY_BARS] = {0};

static void draw_bars(app_state_t *state, int num_bins)
{
    int num_bars = state->main_cols - 2;
    if (num_bars <= 0) return;

    double frame_max = DB_FLOOR;
    for (int i = MIN_EDGE; i < state->buffer_size; i++) {
        double v = state->buffer_data[i];

        if (v < DB_MIN) v = DB_MIN;
        if (v > DB_MAX) v = DB_MAX;
    }

    if (frame_max < SILENCE_THRESHOLD) {
        for (int i = 0; i < num_bars && i < MAX_HISTORY_BARS; i++)
            prev_heights[i] *= SILENCE_DECAY;
        return;
    }

    double min_bin = (double)MIN_EDGE;
    double max_bin = (double)(num_bins - 1);
    double log_min = log10(min_bin);
    double log_max = log10(max_bin);

    static double smooth_max = 0.0;
    smooth_max = smooth_max * (1.0 - SMOOTHING_ALPHA) +
                 frame_max * SMOOTHING_ALPHA;

    for (int bar = 0; bar < num_bars && bar < MAX_HISTORY_BARS; bar++) {

        double t0 = (double)bar / num_bars;
        double t1 = (double)(bar + 1) / num_bars;

        int start = (int)pow(10.0, log_min + t0 * (log_max - log_min));
        int end   = (int)pow(10.0, log_min + t1 * (log_max - log_min));

        if (start < MIN_EDGE) start = MIN_EDGE;
        if (end > num_bins) end = num_bins;
        if (end <= start) end = start + 1;

        double sum = 0.0;
        double weight = 0.0;

        for (int i = start; i < end; i++) {
            double v = state->buffer_data[i];
            if (v < DB_MIN) v = DB_MIN;
            if (v > DB_MAX) v = DB_MAX;

            double w = 1.0;
            sum += v * w;
            weight += w;
        }

        double avg = (weight > 0.0) ? (sum / weight) : DB_FLOOR;

        double norm = (avg - DB_MIN) / (DB_MAX - DB_MIN);

        if (norm < 0.0) norm = 0.0;
        if (norm > 1.0) norm = 1.0;

        int max_height = state->main_rows - 2;
        double height = norm * max_height / BAR_HEIGHT_SCALE;

        if (height < prev_heights[bar]) {
            height = prev_heights[bar] * DECAY_FACTOR;
        }

        prev_heights[bar] = height;

        int draw_x = bar + 1;
        int full = (int)height;

        for (int y = 0; y < full; y++) {
            int draw_y = state->main_rows - BORDER_PADDING - y;
            ncplane_set_bg_rgb(main_win, BG_BAR_COLOR);
            ncplane_putstr_yx(main_win, draw_y, draw_x, " ");
        }
    }

    ncplane_set_bg_default(main_win);
}

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

void graphics_draw(struct notcurses *nc, app_state_t *state, size_t num_bins) 
{
    (void)nc;

    ncplane_erase(main_win);

    ncplane_double_box_sized(main_win, NCSTYLE_NONE, 0, state->main_rows, state->main_cols, 0);
    // ncplane_putstr_yx(main_win, (int)state->y_pos, (int)state->x_pos, "O");
    draw_bars(state, num_bins);

    // ncplane_putstr_aligned(main_win, 1, NCALIGN_CENTER, "wassup");
}

void graphics_shutdown(void)
{
    if (main_win) {
        ncplane_destroy(main_win);
        main_win = NULL;
    }
}
