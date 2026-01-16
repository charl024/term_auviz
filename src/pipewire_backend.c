#include "pipewire_backend.h"
#include "proj_defines.h"

pipewire_capture_t* pipewire_capture_init() 
{
    pipewire_capture_t* capture = malloc(sizeof(pipewire_capture_t));
    if (!capture) {
        return NULL;
    }

    // initialize the capture object
    capture->loop = pw_main_loop_new(NULL);
    capture->stream = pw_stream_new(capture->loop, "Audio", NULL);
    capture->ringbuffer = ringbuffer_create(FFT_SIZE);


    
    return capture;
}

void pipewire_capture_destroy(pipewire_capture_t* capture) 
{
    if (!capture) return;

    pw_stream_destroy(capture->stream);
    pw_main_loop_destroy(capture->loop);
    ringbuffer_destroy(capture->ringbuffer);
    free(capture);
}

int pipewire_capture_get_audio(pipewire_capture_t* capture, float* buffer, size_t size)
{
    if (!capture || !buffer || size == 0) {
        return -1;
    }

    // get audio data from the ringbuffer
    return ringbuffer_read(capture->ringbuffer, buffer, size);
}