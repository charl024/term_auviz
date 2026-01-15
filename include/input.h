#ifndef INPUT_H
#define INPUT_H

#include <notcurses/notcurses.h>
#include "state.h"

void input_poll(struct notcurses *nc, app_state_t *state);

#endif
