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
    acc->max_size = buffer_size;
    return acc;
}

void accumulator_destroy(accumulator_t* acc) 
{
    if (!acc) return;

    free(acc->buffer);
    free(acc);
}

int accumulator_accumulate(accumulator_t* acc, float* sample, size_t input_count) 
{
    if (!acc) return -1;

    while (acc->current_fill < acc->max_size) {
        for (size_t i = 0; i < input_count; i++) {
            if (acc->current_fill < acc->max_size) {
                acc->buffer[acc->current_fill] = sample[i];
                acc->current_fill++;
            } else {
                return 1;
            }
        }
    }

    return 0;
}

void accumulator_move_data_to_out(accumulator_t* acc, float* output_buffer, size_t output_size) 
{
    if (!acc || !output_buffer || output_size == 0) return;

    size_t to_copy = (acc->current_fill < output_size) ? acc->current_fill : output_size;
    for (size_t i = 0; i < to_copy; i++) {
        output_buffer[i] = acc->buffer[i];
    }

    for (size_t i = to_copy; i < acc->current_fill; i++) {
        acc->buffer[i - to_copy] = acc->buffer[i];
    }
    acc->current_fill -= to_copy;
}