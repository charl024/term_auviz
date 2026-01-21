#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdlib.h>
#include <pthread.h>

#define RINGBUFFER_SIZE 4096

typedef struct {
    float *data;
    size_t size;
    size_t write_index;
    size_t read_index;
    pthread_mutex_t *mutex;
} ringbuffer_t;

ringbuffer_t* ringbuffer_create(size_t init_size);
void ringbuffer_destroy(ringbuffer_t* buffer);
int ringbuffer_read(ringbuffer_t* buffer, float* output_data, size_t output_size);
int ringbuffer_write(ringbuffer_t* buffer, float* input_data, size_t input_size);

#endif