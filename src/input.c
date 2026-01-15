#include <notcurses/notcurses.h>
#include "input.h"

void input_poll(struct notcurses *nc, app_state_t *state)
{
    struct ncinput ni;
    struct timespec ts = {0, 0};
    uint32_t id = notcurses_get(nc, &ts, &ni);

    if (id == 0) {
        return;
    }

    switch (id) {
        case 'q':
            state->running = false;
            break;

        case NCKEY_RESIZE:
            notcurses_refresh(nc, NULL, NULL);
            break;

        default:
            break;
    }
}
