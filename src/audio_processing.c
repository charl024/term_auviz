#include <stdio.h>
#include <string.h>
#include <math.h>
#include "audio_processing.h"


audio_processing_t* audio_processing_create(size_t fft_size) 
{
    audio_processing_t* processor = (audio_processing_t*)malloc(sizeof(audio_processing_t));
    if (!processor) {
        fprintf(stderr, "Error allocating audio_processing_t.\n");
        return NULL;
    }

    processor->input = (double *)malloc(sizeof(double) * fft_size);
    processor->output = (double *)malloc(sizeof(double) * fft_size);
    processor->complex_output = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * fft_size);
    processor->plan = fftw_plan_dft_r2c_1d(fft_size, processor->input, processor->complex_output, 0);

    return processor;
}

void audio_processing_destroy(audio_processing_t* processor) 
{
    if (!processor) {
        fprintf(stderr, "audio_processing_t processor is uninitialized!\n");
        return;
    }

    if (processor->input) {
        free(processor->input);
    }

    if (processor->output) {
        free(processor->output);
    }

    if (processor->complex_output) {
        fftw_free(processor->complex_output);
    }

    if (processor->plan) {
        fftw_destroy_plan(processor->plan);
        fftw_cleanup();
    }

    free(processor);
}

void audio_processing_process(audio_processing_t* processor, float* input_data, float* output_data, size_t size) 
{
    if (!processor || !input_data || size == 0) {
        fprintf(stderr, "Invalid audio processing parameters.\n");
        return;
    }

    // Process the audio data
    memcpy(processor->input, input_data, sizeof(float) * size);

    fftw_execute_dft_r2c(processor->plan, processor->input, processor->complex_output);

    for(size_t i = 0; i < size; i++) {
        float a = processor->complex_output[i][0];
        float b = processor->complex_output[i][1];
        float mag = a*a + b*b;
        output_data[i] = sqrt(mag); 
    }
    
}