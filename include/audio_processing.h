#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

#include <stdlib.h>
#include <fftw3.h>

typedef struct {
    size_t fft_size;
    size_t fft_bins;

    double *time_buffer;
    double *freq_db;
    fftw_complex *freq_complex;

    fftw_plan plan;
} audio_processing_t;

audio_processing_t* audio_processing_create(size_t fft_size);
void audio_processing_destroy(audio_processing_t *audio_processor);
void audio_processing_process(audio_processing_t *audio_processor, float *input, double *output_db);

#endif
