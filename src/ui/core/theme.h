#ifndef THEME_H
#define THEME_H

#include <ncurses.h>
#include <stdbool.h>

// 테마 타입
typedef enum {
    THEME_WHITE = 0,    // 기본 화이트 테마
    THEME_HACKER,       // 녹색 해커 테마
    THEME_GOLD,         // 금색 테마
    THEME_SKYBLUE,      // 하늘색 테마
    THEME_PINK,         // 핑크 테마
    THEME_RAINBOW,      // 무지개 테마
    THEME_COUNT         // 테마 개수
} ThemeType;

// 색상 페어 인덱스 (의미론적 매핑)
typedef enum {
    COLOR_PAIR_DEFAULT = 1,      // 기본 텍스트
    COLOR_PAIR_INVERTED = 2,     // 반전 (모달 배경)
    COLOR_PAIR_BLACK_STONE = 3,  // 흑돌 강조
    COLOR_PAIR_WHITE_STONE = 4,  // 백돌 강조
    COLOR_PAIR_SYSTEM = 5,       // 시스템 메시지
    COLOR_PAIR_OPPONENT = 6,     // 상대방 채팅
    COLOR_PAIR_INFO = 7,         // 정보 텍스트
    COLOR_PAIR_DIM = 8           // 비활성/희미한 텍스트
} ColorPairIndex;

// 테마 색상 정의
typedef struct {
    int fg;  // Foreground color
    int bg;  // Background color
} ColorDef;

// 테마 정의
typedef struct {
    const char *name;
    ColorDef pairs[8];  // 8개의 색상 페어
} ThemeDef;

// 테마 매니저
typedef struct {
    ThemeType current_theme;
    bool initialized;
} ThemeManager;

// 전역 테마 매니저
extern ThemeManager g_theme_manager;

// 테마 초기화
bool theme_init(ThemeType theme);

// 테마 변경
bool theme_set(ThemeType theme);

// 현재 테마 가져오기
ThemeType theme_get_current(void);

// 테마 이름 가져오기
const char* theme_get_name(ThemeType theme);

// 테마 개수 가져오기
int theme_get_count(void);

// 저장된 테마 불러오기 (설정 파일에서)
ThemeType theme_load_from_config(void);

// 현재 테마를 설정 파일에 저장
bool theme_save_to_config(void);

#endif // THEME_H
