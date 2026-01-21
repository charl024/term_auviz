#include <stdio.h>
#include "audio_processing.h"


audio_processing_t* audio_processing_create() 
{

}

void audio_processing_destroy(audio_processing_t* processor) 
{

}

void audio_processing_process(audio_processing_t* processor, float* data, size_t size) 
{
    if (!processor || !data || size == 0) {
        fprintf(stderr, "Invalid audio processing parameters.\n");
        return;
    }

    // Process the audio data
}

accumulator_t* accumulator_create(size_t buffer_size) 
{
    accumulator_t* acc = (accumulator_t*)malloc(sizeof(accumulator_t));
    if (!acc) {
        fprintf(stderr, "Error allocating accumulator_t.\n");
        return NULL;
    }

    acc->buffer = (float*)malloc(sizeof(float) * buffer_size);
    if (!acc->buffer) {
        fprintf(stderr, "Error allocating accumulator_t buffer.\n");
        free(acc);
        return NULL;
    }

    acc->current_fill = 0;
    return acc;
}

void accumulator_destroy(accumulator_t* acc) 
{
    if (!acc) return;

    free(acc->buffer);
    free(acc);
}

void accumulator_add_sample(accumulator_t* acc, float* sample, size_t input_count, float* output_buffer, size_t output_size) 
{
    if (!acc) return;

    
}   