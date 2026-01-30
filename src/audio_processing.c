#include <stdio.h>
#include <string.h>
#include <math.h>
#include "audio_processing.h"


audio_processing_t* audio_processing_create(size_t fft_size, size_t fft_out_size) 
{
    audio_processing_t* processor = (audio_processing_t*)malloc(sizeof(audio_processing_t));
    if (!processor) {
        fprintf(stderr, "Error allocating audio_processing_t.\n");
        return NULL;
    }

    processor->input = (double *)malloc(sizeof(double) * fft_size);
    processor->output = (double *)malloc(sizeof(double) * fft_out_size);
    processor->complex_output = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * fft_out_size);
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

static void fft_process(audio_processing_t* processor, size_t input_size, size_t fft_bins) 
{
    fftw_execute(processor->plan);

    for (size_t i = 0; i < fft_bins; i++) {
        double re = processor->complex_output[i][0];
        double im = processor->complex_output[i][1];

        double mag = sqrt(re * re + im * im);
        mag /= (double)input_size;

        double db = 20.0 * log10(mag + 1e-12);

        processor->output[i] = db;
    }
}

static void hanning_window(audio_processing_t* processor, size_t input_size) 
{
    double pi = 3.14159265359;
    for (size_t i = 0; i < input_size; i++) {
        double mult = 0.5 * (1 - cos((2*pi*i)/(input_size - 1)));
        processor->input[i] = processor->input[i] * mult;
    }
}

void audio_processing_process(audio_processing_t* processor, float* input_data, double* output_data, size_t input_size, size_t fft_bins) 
{
    if (!processor || !input_data) {
        fprintf(stderr, "Invalid audio processing parameters.\n");
        return;
    }

    // Process the audio data
    for (size_t i = 0; i < input_size; i++) {
        processor->input[i] = (double)(input_data[i]);
    }

    hanning_window(processor, input_size);
    fft_process(processor, input_size, fft_bins);

    for (size_t i = 0; i < fft_bins; i++) {
        output_data[i] = processor->output[i];
    }
    
}