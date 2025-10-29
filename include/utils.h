#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <time.h>

long utils_now_ms(void);
void utils_sleep_ms(long ms);
void utils_trim_newline(char *str);
int utils_clamp(int value, int min_value, int max_value);
void utils_format_time(int seconds, char *buffer, size_t buffer_len);
int utils_random_int(int min_value, int max_value);
void utils_seed_random(void);

#endif // UTILS_H
