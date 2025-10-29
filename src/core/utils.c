#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

long utils_now_ms(void) {
    struct timespec ts;
#if defined(CLOCK_MONOTONIC)
    clock_gettime(CLOCK_MONOTONIC, &ts);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ts.tv_sec = tv.tv_sec;
    ts.tv_nsec = tv.tv_usec * 1000;
#endif
    return (ts.tv_sec * 1000L) + (ts.tv_nsec / 1000000L);
}

void utils_sleep_ms(long ms) {
    if (ms <= 0) {
        return;
    }
    struct timespec req;
    req.tv_sec = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&req, NULL);
}

void utils_trim_newline(char *str) {
    if (!str) {
        return;
    }
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len--;
    }
}

int utils_clamp(int value, int min_value, int max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

void utils_format_time(int seconds, char *buffer, size_t buffer_len) {
    if (!buffer || buffer_len == 0) {
        return;
    }
    int mins = seconds / 60;
    int secs = seconds % 60;
    snprintf(buffer, buffer_len, "%02d:%02d", mins, secs);
}

int utils_random_int(int min_value, int max_value) {
    if (max_value <= min_value) {
        return min_value;
    }
    int range = max_value - min_value + 1;
    return min_value + (rand() % range);
}

void utils_seed_random(void) {
    static int seeded = 0;
    if (seeded) {
        return;
    }
    seeded = 1;
    srand((unsigned int)time(NULL));
}
