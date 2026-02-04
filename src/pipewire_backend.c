#include "pipewire_backend.h"
#include "pipewire/main-loop.h"
#include "proj_defines.h"
#include <pthread.h>

#define REGISTRY_TIMEOUT_MS 2000

static void on_registry_event(
    void *data,
    uint32_t id,
    uint32_t permissions,
    const char *type,
    uint32_t version,
    const struct spa_dict *props)
{
    pipewire_capture_t *capture = data;

    if (strcmp(type, PW_TYPE_INTERFACE_Node) != 0)
        return;

    const char *media_class = NULL;
    const char *media_role = NULL;
    const char *node_name = NULL;

    const struct spa_dict_item *item;
    spa_dict_for_each(item, props) {
        if (strcmp(item->key, PW_KEY_MEDIA_CLASS) == 0)
            media_class = item->value;
        else if (strcmp(item->key, PW_KEY_MEDIA_ROLE) == 0)
            media_role = item->value;
        else if (strcmp(item->key, PW_KEY_NODE_NAME) == 0)
            node_name = item->value;
    }

    if (!media_class || !media_role || !node_name)
        return;

    if (strcmp(media_class, "Stream/Output/Audio") == 0 && strcmp(media_role, "Music") == 0) {
        

        if (capture->target_node[0] == '\0') {
            
            strncpy(capture->target_node, node_name, sizeof(capture->target_node) - 1);

            // fprintf(
            //     stderr,
            //     "[pipewire] id=%u node=%s media.class=%s media.role=%s\n",
            //     id,
            //     node_name,
            //     media_class,
            //     media_role
            // );
            // fflush(stderr);
        }
        
    }
}


static const struct pw_registry_events registry_events = {
    PW_VERSION_REGISTRY_EVENTS,
    .global = on_registry_event,
};

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
    
    uint32_t stride = spa_buf->datas[0].chunk->stride;
    uint32_t channels = stride / sizeof(float);
    uint32_t frames = spa_buf->datas[0].chunk->size / stride;

    float *samples = spa_buf->datas[0].data;

    for (uint32_t i = 0; i < frames; i++) {
        float mono = 0.0f;
        
        for (uint32_t c = 0; c < channels; c++) {
            mono += samples[i * channels + c];
        }

        mono /= channels;

        mono *= 2.0f;

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
    pw_init(NULL, NULL);

    pipewire_capture_t *capture = calloc(1, sizeof(*capture));

    if (!capture) {
        return NULL;
    }

    // initialize the capture object
    capture->loop = pw_main_loop_new(NULL);
    capture->context = pw_context_new(pw_main_loop_get_loop(capture->loop), NULL, 0);
    capture->core = pw_context_connect(capture->context, NULL, 0);
    capture->registry = pw_core_get_registry(capture->core, PW_VERSION_REGISTRY, 0);

    pw_registry_add_listener(
        capture->registry,
        &capture->registry_listener,
        &registry_events,
        capture);

    int waited = 0;
    while (capture->target_node[0] == '\0' && waited < REGISTRY_TIMEOUT_MS) {
        pw_loop_iterate(pw_main_loop_get_loop(capture->loop), true);
        waited += 50;
    }

    // if (capture->target_node[0] == '\0') {
    //     fprintf(stderr, "Node not found after %d ms\n", REGISTRY_TIMEOUT_MS);
    //     pipewire_capture_destroy(capture);
    //     return NULL;
    // }

    // set props
    struct pw_properties *props = pw_properties_new(
        PW_KEY_MEDIA_TYPE, "Audio",
        PW_KEY_MEDIA_CLASS, "Stream/Input/Audio",
        PW_KEY_MEDIA_ROLE, "Music",
        PW_KEY_TARGET_OBJECT, capture->target_node,
        NULL);

    capture->stream = pw_stream_new_simple(
        pw_main_loop_get_loop(capture->loop),
        "portable-audio-capture",
        props,
        &stream_events,
        capture);

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
            .channels = 0
        ));

    // connect pw stream with parameters
    pw_stream_connect(
        capture->stream,
        PW_DIRECTION_INPUT,
        PW_ID_ANY,
        PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS,
        params, 1);
    
    // allocate thread
    capture->cap_thread = (pthread_t*)malloc(sizeof(pthread_t));
    capture->thread_running = 0;
    
    // return the capture struct
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

    spa_hook_remove(&capture->registry_listener);

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
