#pragma once

/*
 * Timer functions.
 */

#include <time.h>

typedef clock_t nc_timepoint;

static inline nc_timepoint nc_timer_current() {
    return clock();
}

static inline int nc_timer_elapsed_ms(nc_timepoint t) {
    return (nc_timer_current() - t) / (CLOCKS_PER_SEC / 1000);
}

static inline float nc_timer_elapsed_s(nc_timepoint t) {
    return nc_timer_elapsed_ms(t) / 1000.0f;
}

static inline nc_timepoint nc_timer_futurems(int ms) {
    return nc_timer_current() + ms * (CLOCKS_PER_SEC / 1000);
}
