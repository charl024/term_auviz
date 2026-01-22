#include <notcurses/notcurses.h>
#include "app.h"
#include "graphics.h"
#include "input.h"
#include "state.h"
#include "time_helper.h"
#include "visualizer.h"
#include "pipewire_backend.h"
#include "proj_defines.h"
#include "audio_processing.h"

int app_run()
{
    // init notcurses
    struct notcurses_options opts = {0};
    struct notcurses *nc = notcurses_init(&opts, NULL);
    
    if (!nc) return 1;

    // set state to running
    app_state_t state = {
        .running = 1,

        .x_pos = 5,
        .y_pos = 5,
        .x_vel = 60.0,
        .y_vel = 60.0
    };

    // initialize and launch pipewire thread
    pipewire_capture_t *audio_cap = pipewire_capture_create();
    float *audio_buffer = (float*)malloc(sizeof(float) * FFT_SIZE);

    pipewire_capture_run(audio_cap);

    // initialize audio processing
    audio_processing_t *audio_proc = audio_processing_create();
    accumulator_t *acc = accumulator_create(FFT_SIZE);


    graphics_init(nc, &state);

    float processed_buffer[FFT_SIZE];

    // time setup
    struct timespec last, now;
    clock_gettime(CLOCK_MONOTONIC, &last);

    while (state.running) {
        input_poll(nc, &state);

        clock_gettime(CLOCK_MONOTONIC, &now);
        long elapsed_ns = diff_ns(&now, &last);

        if (elapsed_ns >= FRAME_TIME_NS) {

            int bytes_read = pipewire_capture_get_audio(audio_cap, audio_buffer, FFT_SIZE);

            if (bytes_read > 0) {
                int r = accumulator_accumulate(acc, audio_buffer, bytes_read);
                if (r) {
                    accumulator_move_data_to_out(acc, processed_buffer, FFT_SIZE);
                    printf("data\n");
                }
            }

            update_visual_state(&state, elapsed_ns);
            graphics_draw(nc, &state);
            notcurses_render(nc);
            last = now;
        } else {
            struct timespec sleep_time = {
                .tv_sec = 0,
                .tv_nsec = FRAME_TIME_NS - elapsed_ns,
            };
            nanosleep(&sleep_time, NULL);
        }

        
    }

    graphics_shutdown();
    notcurses_stop(nc);
    pipewire_capture_destroy(audio_cap);
    audio_processing_destroy(audio_proc);
    accumulator_destroy(acc);
    free(audio_buffer);
    return 0;
}