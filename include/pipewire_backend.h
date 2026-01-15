#ifndef PIPEWIRE_BACKEND_H
#define PIPEWIRE_BACKEND_H

#include <pipewire/pipewire.h>
#include <spa/param/audio/format-util.h>
#include <math.h>

typedef struct {
	struct pw_main_loop *loop;
	struct pw_stream *stream;
} pipewire_capture_t;

pipewire_capture_t* pipewire_capture_create();
void pipewire_capture_destroy();


#endif
