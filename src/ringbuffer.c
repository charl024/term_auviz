#include "ringbuffer.h"

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

    pthread_mutex_init(buffer->mutex, NULL);
    
    buffer->size = size;
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

    free(buffer->data);
    free(buffer);
}

void ringbuffer_read(ringbuffer_t* buffer, void* output_data, size_t output_size) 
{
    size_t r, w;
    pthread_mutex_lock(buffer->mutex);
    r = buffer->read_index;
    w = buffer->write_index;
    pthread_mutex_unlock(buffer->mutex);

    size_t available = w - r;
    size_t max_n = available < output_size ? available : output_size;

    // TODO read buffer data into output

}

void ringbuffer_write(ringbuffer_t* buffer, void* input_data, size_t input_size) 
{

}