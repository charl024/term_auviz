#include "pipewire_backend.h"
#include "pipewire/main-loop.h"
#include "proj_defines.h"
#include <pthread.h>

static void on_process(void *data) 
{
    pipewire_capture_t* capture = data;

    struct pw_buffer *buf = pw_stream_dequeue_buffer(capture->stream);
     
    if (!buf) return;

    struct spa_buffer *spa_buf = buf->buffer;
    
    if (spa_buf->datas[0].data == NULL) {
        pw_stream_queue_buffer(capture->stream, buf);
        return;
    }
    
    float *samples = spa_buf->datas[0].data;
    uint32_t frames = spa_buf->datas[0].chunk->size / (sizeof(float) * 2);

    for (uint32_t i = 0; i < frames; i++) {
        float mono = 0.5f * (samples[2*i] + samples[2*i + 1]);
        ringbuffer_write(capture->ringbuffer, &mono, 1);
    }

    pw_stream_queue_buffer(capture->stream, buf);
}
    

static const struct pw_stream_events stream_events = {
    PW_VERSION_STREAM_EVENTS,
    .process = on_process,
};

static void* pipewire_capture_launch_loop(void* data) 
{
    pipewire_capture_t* capture = (pipewire_capture_t*)data;
    pw_main_loop_run(capture->loop);

    return NULL;
}

void pipewire_capture_run(pipewire_capture_t* capture) 
{
    // create pthread that runs pw loop
    capture->thread_running = 1;
    pthread_create(capture->cap_thread, NULL, pipewire_capture_launch_loop, (void*)capture);
}

pipewire_capture_t* pipewire_capture_create() 
{
    pipewire_capture_t* capture = malloc(sizeof(pipewire_capture_t));
    if (!capture) {
        return NULL;
    }

	struct pw_properties* props = pw_properties_new(
			PW_KEY_MEDIA_TYPE, "Audio",
			PW_KEY_MEDIA_CATEGORY, "Capture",
			PW_KEY_MEDIA_ROLE, "Music",
			NULL);

    pw_properties_set(props, PW_KEY_STREAM_CAPTURE_SINK, "true");

    // initialize the capture object
    capture->loop = pw_main_loop_new(NULL);
    capture->stream = pw_stream_new_simple(
			pw_main_loop_get_loop(capture->loop),
			"native-audio_capture", 
			props,
			&stream_events,
			capture);

    // pw_properties_free(props);

	// initialize ringbuffer
    capture->ringbuffer = ringbuffer_create(FFT_SIZE);

    uint8_t param_buf[1024];
    struct spa_pod_builder builder = SPA_POD_BUILDER_INIT(param_buf, sizeof(param_buf));
    const struct spa_pod *params[1];

    // parameter setup
    params[0] = spa_format_audio_raw_build(
        &builder,
        SPA_PARAM_EnumFormat,
        &SPA_AUDIO_INFO_RAW_INIT(
            .format   = SPA_AUDIO_FORMAT_F32,
            .rate     = 48000,
            .channels = 2
        ));

    // connect pw stream with parameters
    pw_stream_connect(
        capture->stream,
        PW_DIRECTION_INPUT,
        PW_ID_ANY,
        PW_STREAM_FLAG_AUTOCONNECT |
        PW_STREAM_FLAG_MAP_BUFFERS |
        PW_STREAM_FLAG_RT_PROCESS,
        params, 1);
    
    // allocate thread
    capture->cap_thread = (pthread_t*)malloc(sizeof(pthread_t));
    capture->thread_running = 0;
    
    // return the capture struct, do not start the pw loop
    return capture;
}


void pipewire_capture_destroy(pipewire_capture_t* capture)
{
    if (!capture) return;

    if (capture->loop) {
        pw_main_loop_quit(capture->loop);
    }

    if (capture->thread_running && capture->cap_thread) {
        pw_main_loop_quit(capture->loop);
        pthread_join(*capture->cap_thread, NULL);
        capture->thread_running = 0;
    }

    if (capture->cap_thread) {
        free(capture->cap_thread);
        capture->cap_thread = NULL;
    }

    if (capture->stream) {
        pw_stream_destroy(capture->stream);
        capture->stream = NULL;
    }

    if (capture->loop) {
        pw_main_loop_destroy(capture->loop);
        capture->loop = NULL;
    }

    if (capture->ringbuffer) {
        ringbuffer_destroy(capture->ringbuffer);
        capture->ringbuffer = NULL;
    }

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
