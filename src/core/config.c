#include "config.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void config_set_defaults(config_t *cfg) {
    if (!cfg) {
        return;
    }
    memset(cfg->nickname, 0, sizeof(cfg->nickname));
    snprintf(cfg->nickname, sizeof(cfg->nickname), "Player");
    cfg->default_ai_level = AI_LEVEL_MEDIUM;
    cfg->turn_time_seconds = 30;
    cfg->default_port = 4242;
}

static ai_level_t parse_ai_level(const char *value) {
    if (!value) {
        return AI_LEVEL_MEDIUM;
    }
    if (strcasecmp(value, "easy") == 0) {
        return AI_LEVEL_EASY;
    }
    if (strcasecmp(value, "medium") == 0) {
        return AI_LEVEL_MEDIUM;
    }
    if (strcasecmp(value, "hard") == 0) {
        return AI_LEVEL_HARD;
    }
    return AI_LEVEL_MEDIUM;
}

int config_load(config_t *cfg, const char *path) {
    if (!cfg) {
        return -1;
    }

    config_set_defaults(cfg);

    const char *file_path = path ? path : CONFIG_DEFAULT_FILE;
    FILE *fp = fopen(file_path, "r");
    if (!fp) {
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n') {
            continue;
        }
        char *eq = strchr(line, '=');
        if (!eq) {
            continue;
        }
        *eq = '\0';
        char *key = line;
        char *value = eq + 1;

        // Trim whitespace
        while (*key && isspace((unsigned char)*key)) {
            key++;
        }
        char *key_end = key + strlen(key);
        while (key_end > key && isspace((unsigned char)key_end[-1])) {
            key_end--;
        }
        *key_end = '\0';

        while (*value && isspace((unsigned char)*value)) {
            value++;
        }
        char *value_end = value + strlen(value);
        while (value_end > value && isspace((unsigned char)value_end[-1])) {
            value_end--;
        }
        *value_end = '\0';

        if (strcmp(key, "nickname") == 0) {
            strncpy(cfg->nickname, value, sizeof(cfg->nickname) - 1);
        } else if (strcmp(key, "ai_difficulty") == 0) {
            cfg->default_ai_level = parse_ai_level(value);
        } else if (strcmp(key, "turn_time") == 0) {
            cfg->turn_time_seconds = atoi(value);
            if (cfg->turn_time_seconds <= 0) {
                cfg->turn_time_seconds = 30;
            }
        } else if (strcmp(key, "server_port") == 0) {
            cfg->default_port = (uint16_t)atoi(value);
        }
    }

    fclose(fp);
    return 0;
}

int config_save(const config_t *cfg, const char *path) {
    if (!cfg) {
        return -1;
    }
    const char *file_path = path ? path : CONFIG_DEFAULT_FILE;
    FILE *fp = fopen(file_path, "w");
    if (!fp) {
        return -1;
    }
    fprintf(fp, "# Gomoku configuration\n");
    fprintf(fp, "nickname=%s\n", cfg->nickname);
    fprintf(fp, "ai_difficulty=%s\n", config_ai_level_name(cfg->default_ai_level));
    fprintf(fp, "turn_time=%d\n", cfg->turn_time_seconds);
    fprintf(fp, "server_port=%u\n", cfg->default_port);
    fclose(fp);
    return 0;
}

const char *config_ai_level_name(ai_level_t level) {
    switch (level) {
        case AI_LEVEL_EASY:
            return "easy";
        case AI_LEVEL_MEDIUM:
            return "medium";
        case AI_LEVEL_HARD:
            return "hard";
        default:
            return "medium";
    }
}
