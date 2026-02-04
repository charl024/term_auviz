#ifndef STATE_H
#define STATE_H

typedef struct {
    int running;

    int term_rows;
    int term_cols;
    int main_rows;
    int main_cols;

    int last_term_rows;
    int last_term_cols;


    int min_x;
    int min_y;
    int max_x;
    int max_y;

    int needs_resize;

    int buffer_size;
    double* buffer_data;
    
} app_state_t;

#endif
