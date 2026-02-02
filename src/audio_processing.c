#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "audio_processing.h"

#define DB_EPSILON 1e-12

static inline void apply_hanning(double *buf, size_t n)
{
    const double two_pi = 2.0 * M_PI;
    for (size_t i = 0; i < n; i++) {
        double w = 0.5 * (1.0 - cos(two_pi * i / (n - 1)));
        buf[i] *= w;
    }
}

audio_processing_t* audio_processing_create(size_t fft_size) 
{
    audio_processing_t* audio_processor = calloc(1, sizeof(*audio_processor));
    if (!audio_processor) {
        fprintf(stderr, "Error allocating audio_processing_t.\n");
        return NULL;
    }

    audio_processor->fft_size = fft_size;
    audio_processor->fft_bins = fft_size / 2 + 1;
    audio_processor->time_buffer = (double*)malloc(sizeof(double) * audio_processor->fft_size);
    audio_processor->freq_db = (double*)malloc(sizeof(double) * audio_processor->fft_bins);
    audio_processor->freq_complex = fftw_malloc(sizeof(fftw_complex) * audio_processor->fft_bins);

    if (!audio_processor->time_buffer || !audio_processor->freq_db || !audio_processor->freq_complex) {
        audio_processing_destroy(audio_processor);
        return NULL;
    }

    audio_processor->plan = fftw_plan_dft_r2c_1d(
        fft_size,
        audio_processor->time_buffer,
        audio_processor->freq_complex,
        FFTW_MEASURE
    );

    if (!audio_processor->plan) {
        audio_processing_destroy(audio_processor);
        return NULL;
    }

    return audio_processor;
}

void audio_processing_destroy(audio_processing_t* audio_processor) 
{
    if (!audio_processor) return;

    if (audio_processor->plan) fftw_destroy_plan(audio_processor->plan);

    fftw_free(audio_processor->freq_complex);
    free(audio_processor->freq_db);
    free(audio_processor->time_buffer);

    free(audio_processor);
}

void audio_processing_process(
    audio_processing_t *audio_processor,
    float *input,
    double *output_db
)
{
    for (size_t i = 0; i < audio_processor->fft_size; i++) {
        audio_processor->time_buffer[i] = (double)input[i];
    }
    apply_hanning(audio_processor->time_buffer, audio_processor->fft_size);

    fftw_execute(audio_processor->plan);

    const double norm = 1.0 / (double)audio_processor->fft_size;

    for (size_t i = 0; i < audio_processor->fft_bins; i++) {
        double re = audio_processor->freq_complex[i][0];
        double im = audio_processor->freq_complex[i][1];

        double mag = sqrt(re * re + im * im) * norm;
        audio_processor->freq_db[i] = 20.0 * log10(mag + DB_EPSILON);
    }

    memcpy(output_db, audio_processor->freq_db, sizeof(double) * audio_processor->fft_bins);
}