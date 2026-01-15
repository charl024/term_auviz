#ifndef STATE_H
#define STATE_H

typedef struct {
    int running;

    int term_rows;
    int term_cols;
    int main_rows;
    int main_cols;

    int min_x;
    int min_y;
    int max_x;
    int max_y;

    double x_pos;
    double y_pos;
    double x_vel;
    double y_vel;
    
} app_state_t;

#endif
