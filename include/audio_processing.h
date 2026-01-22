#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

#include <stdlib.h>

typedef struct {
    
} audio_processing_t;

audio_processing_t* audio_processing_create();
void audio_processing_destroy(audio_processing_t* processor);
void audio_processing_process(audio_processing_t* processor, float* data, size_t size);

typedef struct {
    float* buffer;
    size_t current_fill;
    size_t max_size;
} accumulator_t;

accumulator_t* accumulator_create(size_t buffer_size);
void accumulator_destroy(accumulator_t* acc);
int accumulator_accumulate(accumulator_t* acc, float* sample, size_t input_count);
void accumulator_move_data_to_out(accumulator_t* acc, float* output_buffer, size_t output_size);

#endif