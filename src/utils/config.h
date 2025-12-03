#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#define CONFIG_FILE ".gomoku-config"
#define CONFIG_MAX_LINE 256

// 설정 항목
typedef struct {
    int theme;              // 선택된 테마 (ThemeType)
} GameConfig;

// 설정 파일 로드
bool config_load(GameConfig *config);

// 설정 파일 저장
bool config_save(const GameConfig *config);

// 기본 설정으로 초기화
void config_init_default(GameConfig *config);

#endif // CONFIG_H
