#ifndef PIPEWIRE_BACKEND_H
#define PIPEWIRE_BACKEND_H

#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
#include <stdlib.h>
#include <pthread.h>
#include "ringbuffer.h"

typedef struct {
	struct pw_main_loop *loop;
	struct pw_stream *stream;
	struct spa_pod_builder *spap_builer;
	ringbuffer_t *ringbuffer;

	int thread_running;
	pthread_t *cap_thread;
} pipewire_capture_t;

pipewire_capture_t* pipewire_capture_create();
void pipewire_capture_run(pipewire_capture_t* capture);
void pipewire_capture_destroy(pipewire_capture_t* capture);
int pipewire_capture_get_audio(pipewire_capture_t* capture, float* buffer, size_t size);


#endif
