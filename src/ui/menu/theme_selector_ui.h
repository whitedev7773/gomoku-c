#ifndef THEME_SELECTOR_UI_H
#define THEME_SELECTOR_UI_H

#include "../core/theme.h"
#include <ncurses.h>

typedef struct
{
    int selected_theme;
    int theme_count;
} ThemeSelectorUI;

// 초기화
void theme_selector_init(ThemeSelectorUI *selector);

// 렌더링
void theme_selector_render(WINDOW *win, const ThemeSelectorUI *selector);

// 선택 이동
void theme_selector_move(ThemeSelectorUI *selector, int direction);

// 선택된 테마 가져오기
ThemeType theme_selector_get_selected(const ThemeSelectorUI *selector);

// 테마 선택 화면 실행 (테마를 선택하고 적용)
ThemeType theme_selector_run(void);

#endif // THEME_SELECTOR_UI_H
