#include "timer.h"

#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE < 199309L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

#include <string.h>

static void read_now(struct timespec *out) {
#if defined(CLOCK_MONOTONIC)
    clock_gettime(CLOCK_MONOTONIC, out);
#else
    clock_gettime(CLOCK_REALTIME, out);
#endif
}

void timer_start(turn_timer_t *timer, int total_seconds) {
    if (!timer) {
        return;
    }
    timer->total_seconds = total_seconds;
    read_now(&timer->start_time);
    timer->running = true;
}

void timer_stop(turn_timer_t *timer) {
    if (!timer) {
        return;
    }
    timer->running = false;
}

static int diff_seconds(const struct timespec *start, const struct timespec *end) {
    time_t sec = end->tv_sec - start->tv_sec;
    long nsec = end->tv_nsec - start->tv_nsec;
    if (nsec < 0) {
        sec -= 1;
        nsec += 1000000000L;
    }
    return (int)(sec + (nsec / 1000000000.0));
}

int timer_elapsed(const turn_timer_t *timer) {
    if (!timer || !timer->running) {
        return 0;
    }
    struct timespec now;
    read_now(&now);
    return diff_seconds(&timer->start_time, &now);
}

int timer_remaining(const turn_timer_t *timer) {
    if (!timer) {
        return 0;
    }
    if (!timer->running) {
        return timer->total_seconds;
    }
    int remaining = timer->total_seconds - timer_elapsed(timer);
    if (remaining < 0) {
        remaining = 0;
    }
    return remaining;
}

bool timer_expired(const turn_timer_t *timer) {
    if (!timer) {
        return false;
    }
    if (timer->total_seconds <= 0) {
        return false;
    }
    return timer_remaining(timer) <= 0;
}
