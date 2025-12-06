#include "theme.h"
#include "../../utils/config.h"
#include <string.h>

// 전역 테마 매니저
ThemeManager g_theme_manager = {
    .current_theme = THEME_WHITE,
    .initialized = false};

// 테마 정의
static const ThemeDef themes[THEME_COUNT] = {
    // THEME_WHITE (기본)
    {
        .name = "White",
        .pairs = {
            {COLOR_WHITE, COLOR_BLACK},  // DEFAULT
            {COLOR_BLACK, COLOR_WHITE},  // INVERTED
            {COLOR_RED, COLOR_BLACK},    // BLACK_STONE
            {COLOR_GREEN, COLOR_BLACK},  // WHITE_STONE
            {COLOR_YELLOW, COLOR_BLACK}, // SYSTEM
            {COLOR_BLUE, COLOR_BLACK},   // OPPONENT
            {COLOR_CYAN, COLOR_BLACK},   // INFO
            {COLOR_WHITE, COLOR_BLACK}   // DIM
        }},

    // THEME_HACKER (녹색 해커 스타일)
    {.name = "Hacker", .pairs = {
                           {COLOR_GREEN, COLOR_BLACK},  // DEFAULT
                           {COLOR_BLACK, COLOR_GREEN},  // INVERTED
                           {COLOR_WHITE, COLOR_BLACK},  // BLACK_STONE
                           {COLOR_CYAN, COLOR_BLACK},   // WHITE_STONE
                           {COLOR_YELLOW, COLOR_BLACK}, // SYSTEM
                           {COLOR_GREEN, COLOR_BLACK},  // OPPONENT
                           {COLOR_CYAN, COLOR_BLACK},   // INFO
                           {COLOR_GREEN, COLOR_BLACK}   // DIM
                       }},

    // THEME_GOLD (금색)
    {.name = "Gold", .pairs = {
                         {COLOR_YELLOW, COLOR_BLACK}, // DEFAULT
                         {COLOR_BLACK, COLOR_YELLOW}, // INVERTED
                         {COLOR_RED, COLOR_BLACK},    // BLACK_STONE
                         {COLOR_WHITE, COLOR_BLACK},  // WHITE_STONE
                         {COLOR_YELLOW, COLOR_BLACK}, // SYSTEM
                         {COLOR_CYAN, COLOR_BLACK},   // OPPONENT
                         {COLOR_YELLOW, COLOR_BLACK}, // INFO
                         {COLOR_YELLOW, COLOR_BLACK}  // DIM
                     }},

    // THEME_SKYBLUE (하늘색)
    {.name = "Skyblue", .pairs = {
                            {COLOR_CYAN, COLOR_BLACK},    // DEFAULT
                            {COLOR_BLACK, COLOR_CYAN},    // INVERTED
                            {COLOR_MAGENTA, COLOR_BLACK}, // BLACK_STONE
                            {COLOR_YELLOW, COLOR_BLACK},  // WHITE_STONE
                            {COLOR_YELLOW, COLOR_BLACK},  // SYSTEM
                            {COLOR_BLUE, COLOR_BLACK},    // OPPONENT
                            {COLOR_CYAN, COLOR_BLACK},    // INFO
                            {COLOR_CYAN, COLOR_BLACK}     // DIM
                        }},

    // THEME_PINK (핑크)
    {.name = "Pink", .pairs = {
                         {COLOR_MAGENTA, COLOR_BLACK}, // DEFAULT
                         {COLOR_BLACK, COLOR_MAGENTA}, // INVERTED
                         {COLOR_RED, COLOR_BLACK},     // BLACK_STONE
                         {COLOR_WHITE, COLOR_BLACK},   // WHITE_STONE
                         {COLOR_YELLOW, COLOR_BLACK},  // SYSTEM
                         {COLOR_MAGENTA, COLOR_BLACK}, // OPPONENT
                         {COLOR_CYAN, COLOR_BLACK},    // INFO
                         {COLOR_MAGENTA, COLOR_BLACK}  // DIM
                     }}};

// 테마 초기화
bool theme_init(ThemeType theme)
{
    if (theme < 0 || theme >= THEME_COUNT)
    {
        return false;
    }

    if (!has_colors())
    {
        return false;
    }

    start_color();

    // 선택한 테마의 색상 페어 초기화
    const ThemeDef *theme_def = &themes[theme];
    for (int i = 0; i < 8; i++)
    {
        init_pair(i + 1, theme_def->pairs[i].fg, theme_def->pairs[i].bg);
    }

    g_theme_manager.current_theme = theme;
    g_theme_manager.initialized = true;

    return true;
}

// 테마 변경
bool theme_set(ThemeType theme)
{
    if (theme < 0 || theme >= THEME_COUNT)
    {
        return false;
    }

    if (!g_theme_manager.initialized)
    {
        return theme_init(theme);
    }

    // 색상 페어 재초기화
    const ThemeDef *theme_def = &themes[theme];
    for (int i = 0; i < 8; i++)
    {
        init_pair(i + 1, theme_def->pairs[i].fg, theme_def->pairs[i].bg);
    }

    g_theme_manager.current_theme = theme;

    return true;
}

// 현재 테마 가져오기
ThemeType theme_get_current(void)
{
    return g_theme_manager.current_theme;
}

// 테마 이름 가져오기
const char *theme_get_name(ThemeType theme)
{
    if (theme < 0 || theme >= THEME_COUNT)
    {
        return "Unknown";
    }
    return themes[theme].name;
}

// 테마 개수 가져오기
int theme_get_count(void)
{
    return THEME_COUNT;
}

// 저장된 테마 불러오기 (설정 파일에서)
ThemeType theme_load_from_config(void)
{
    GameConfig config;

    if (config_load(&config))
    {
        // 설정 파일에서 테마 로드 성공
        if (config.theme >= 0 && config.theme < THEME_COUNT)
        {
            return (ThemeType)config.theme;
        }
    }

    // 로드 실패 시 기본 테마 반환
    return THEME_WHITE;
}

// 현재 테마를 설정 파일에 저장
bool theme_save_to_config(void)
{
    GameConfig config;
    config.theme = g_theme_manager.current_theme;

    return config_save(&config);
}
