#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <time.h>

typedef struct {
    int total_seconds;
    struct timespec start_time;
    bool running;
} turn_timer_t;

void timer_start(turn_timer_t *timer, int total_seconds);
void timer_stop(turn_timer_t *timer);
int timer_elapsed(const turn_timer_t *timer);
int timer_remaining(const turn_timer_t *timer);
bool timer_expired(const turn_timer_t *timer);

#endif // TIMER_H
