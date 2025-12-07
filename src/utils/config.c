#include "config.h"
#include "../ui/core/theme.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 기본 설정으로 초기화
void config_init_default(GameConfig *config)
{
    if (!config)
        return;

    config->theme = 0; // THEME_WHITE (기본)
}

// 설정 파일 로드
bool config_load(GameConfig *config)
{
    if (!config)
        return false;

    // 기본값으로 초기화
    config_init_default(config);

    FILE *file = fopen(CONFIG_FILE, "r");
    if (!file)
    {
        // 설정 파일이 없으면 기본값 사용
        return false;
    }

    char line[CONFIG_MAX_LINE];
    while (fgets(line, sizeof(line), file))
    {
        // 주석과 빈 줄 무시
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
        {
            continue;
        }

        // key=value 파싱
        char *equals = strchr(line, '=');
        if (!equals)
            continue;

        *equals = '\0';
        char *key = line;
        char *value = equals + 1;

        // 개행 문자 제거
        value[strcspn(value, "\r\n")] = '\0';

        // 설정 값 파싱
        if (strcmp(key, "theme") == 0)
        {
            char *endptr;
            long theme = strtol(value, &endptr, 10);
            if (*endptr == '\0' && theme >= 0 && theme < THEME_COUNT)
            {
                config->theme = (int)theme;
            }
        }
    }

    fclose(file);
    return true;
}

// 설정 파일 저장
bool config_save(const GameConfig *config)
{
    if (!config)
        return false;

    FILE *file = fopen(CONFIG_FILE, "w");
    if (!file)
    {
        return false;
    }

    fprintf(file, "# Gomoku-C Configuration File\n");
    fprintf(file, "# This file is auto-generated. Edit with care.\n\n");
    fprintf(file, "# Selected theme (0-5)\n");
    fprintf(file, "# 0: White, 1: Hacker, 2: Gold, 3: Skyblue, 4: Pink\n");
    fprintf(file, "theme=%d\n", config->theme);

    fclose(file);
    return true;
}
