#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdlib.h>

#define RINGBUFFER_SIZE 4096

typedef struct {
    float *buffer;
    size_t size;
    size_t write_index;
    size_t read_index;
} ringbuffer_t;

ringbuffer_t* ringbuffer_create();
void ringbuffer_destroy(ringbuffer_t* buffer);

#endif