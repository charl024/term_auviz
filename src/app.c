#include <notcurses/notcurses.h>
#include <locale.h>

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
    setlocale(LC_ALL, "");
    // init notcurses
    struct notcurses_options opts = {0};
    struct notcurses *nc = notcurses_init(&opts, NULL);
    
    if (!nc) return 1;

    // initialize and launch pipewire thread
    pipewire_capture_t *audio_cap = pipewire_capture_create();
    float *input_buffer = (float*)malloc(sizeof(float) * FFT_SIZE);
    double *output_buffer = (double*)malloc(sizeof(double) * FFT_BINS);
    pipewire_capture_run(audio_cap);

    // set state to running
    app_state_t state = {
        .running = 1,
        .needs_resize = 0,
        .buffer_size = FFT_BINS,
        .buffer_data = output_buffer
    };

    // initialize audio processing
    audio_processing_t *audio_processor = audio_processing_create(FFT_SIZE);

    graphics_init(nc, &state);

    // time setup
    struct timespec last, now;
    clock_gettime(CLOCK_MONOTONIC, &last);

    while (state.running) {
        input_poll(nc, &state);

        clock_gettime(CLOCK_MONOTONIC, &now);
        long elapsed_ns = diff_ns(&now, &last);

        if (elapsed_ns >= FRAME_TIME_NS) {

            int bytes_read = pipewire_capture_get_audio(audio_cap, input_buffer, FFT_SIZE);

            if (bytes_read == FFT_SIZE) {
                audio_processing_process(audio_processor, input_buffer, output_buffer);
            }

            update_visual_state(&state, elapsed_ns);

            graphics_draw(nc, &state, FFT_BINS);
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
    audio_processing_destroy(audio_processor);
    free(input_buffer);
    free(output_buffer);
    return 0;
}