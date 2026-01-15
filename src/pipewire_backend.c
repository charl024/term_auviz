#include "pipewire_backend.h"

pipewire_capture_t* pipewire_capture_init() 
{
    pipewire_capture_t* capture = malloc(sizeof(pipewire_capture_t));
    if (!capture) {
        return NULL;
    }
    // Initialize the capture object
    return capture;
}

void pipewire_capture_destroy() 
{

}