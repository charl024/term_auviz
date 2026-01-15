#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <notcurses/notcurses.h>
#include "state.h"

void graphics_init(struct notcurses *nc, app_state_t *state);
void graphics_draw(struct notcurses *nc, app_state_t *state);
void graphics_shutdown(void);


#endif