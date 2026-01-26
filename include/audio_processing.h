#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

#include <stdlib.h>
#include <fftw3.h>

typedef struct {
    fftw_plan plan;
    double* input;
    double* output;
    fftw_complex *complex_output;
} audio_processing_t;

audio_processing_t* audio_processing_create(size_t fft_size);
void audio_processing_destroy(audio_processing_t* processor);
void audio_processing_process(audio_processing_t* processor, float* input_data, float* output_data, size_t size);

#endif