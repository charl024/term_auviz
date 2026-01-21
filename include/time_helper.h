#ifndef TIME_HELPER_H
#define TIME_HELPER_H

#include <time.h>

#define TARGET_FPS 60
#define FRAME_TIME_NS (1000000000L / TARGET_FPS)

static inline long
timespec_to_ns(const struct timespec *ts)
{
    return ts->tv_sec * 1000000000L + ts->tv_nsec;
}

static inline long
diff_ns(const struct timespec *a, const struct timespec *b)
{
    return timespec_to_ns(a) - timespec_to_ns(b);
}

#endif
