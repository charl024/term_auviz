#include "ringbuffer.h"
#include <stdio.h>

ringbuffer_t* ringbuffer_create() 
{
    ringbuffer_t* buffer = (ringbuffer_t*)calloc(1, sizeof(ringbuffer_t));

    if (!buffer) {
        fprintf(stderr, "Error allocating ringbuffer_t./n");
        return NULL;
    }

    buffer->data = (float*)malloc(sizeof(float) * RINGBUFFER_SIZE);

    if (!buffer) {
        fprintf(stderr, "Error allocating ringbuffer_t data./n");
        return NULL;
    }

    buffer->mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(buffer->mutex, NULL);
    
    buffer->size = RINGBUFFER_SIZE;
    buffer->write_index = 0;
    buffer->read_index = 0;

    return buffer;
}

void ringbuffer_destroy(ringbuffer_t* buffer) 
{
    if (!buffer) {
        fprintf(stderr, "Error destroying ringbuffer_t, uninitialized./n");
        return;
    }

    if (!buffer->data) {
        fprintf(stderr, "Error allocating ringbuffer_t, data error./n");
        return;
    }

    pthread_mutex_destroy(buffer->mutex);
    free(buffer->data);
    free(buffer);
}

int ringbuffer_read(ringbuffer_t* buffer, float* output_data, size_t output_size) 
{
    pthread_mutex_lock(buffer->mutex);

    size_t r = buffer->read_index;
    size_t w = buffer->write_index;

    size_t available = w - r;
    size_t to_read = available < output_size ? available : output_size;

    for (size_t i = 0; i < to_read; i++) {
        output_data[i] = buffer->data[(r + i) % RINGBUFFER_SIZE];
    }

    buffer->read_index = r + to_read;

    pthread_mutex_unlock(buffer->mutex);
    return to_read;
}

int ringbuffer_write(ringbuffer_t* buffer, float* input_data, size_t input_size)
{
    pthread_mutex_lock(buffer->mutex);

    size_t w = buffer->write_index;
    size_t r = buffer->read_index;
    size_t available = RINGBUFFER_SIZE - (w - r);

    size_t to_write = input_size < available ? input_size : available;

    for (size_t i = 0; i < to_write; i++) {
        buffer->data[(w + i) % RINGBUFFER_SIZE] = input_data[i];
    }

    buffer->write_index = w + to_write;

    pthread_mutex_unlock(buffer->mutex);
    return to_write;
}