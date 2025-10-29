#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#define CONFIG_MAX_NICKNAME 32
#define CONFIG_DEFAULT_FILE "assets/default.cfg"

typedef enum {
    AI_LEVEL_EASY = 0,
    AI_LEVEL_MEDIUM = 1,
    AI_LEVEL_HARD = 2
} ai_level_t;

typedef struct {
    char nickname[CONFIG_MAX_NICKNAME];
    ai_level_t default_ai_level;
    int turn_time_seconds;
    uint16_t default_port;
} config_t;

int config_load(config_t *cfg, const char *path);
int config_save(const config_t *cfg, const char *path);
const char *config_ai_level_name(ai_level_t level);

#endif // CONFIG_H
