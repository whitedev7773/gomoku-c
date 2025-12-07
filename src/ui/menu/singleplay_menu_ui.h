#ifndef SINGLEPLAY_MENU_UI_H
#define SINGLEPLAY_MENU_UI_H

#include <ncurses.h>
#include <stdbool.h>
#include "../../game/ai/ai_engine.h"
#include "../../game/core/board.h"

// 난이도 선택 UI
typedef struct
{
    int selected;      // 0: Easy, 1: Hard
    int option_count;  // 2
    bool needs_render; // 렌더링 필요 여부
} DifficultySelectUI;

// 규칙 선택 UI
typedef struct
{
    int selected;      // 0: Standard, 1: Renju
    int option_count;  // 2
    bool needs_render; // 렌더링 필요 여부
} RuleSelectUI;

// 난이도 선택 UI 초기화
void difficulty_select_ui_init(DifficultySelectUI *ui);

// 난이도 선택 UI 렌더링
void difficulty_select_ui_render(WINDOW *win, DifficultySelectUI *ui);

// 난이도 선택 이동 (좌우/상하 지원)
void difficulty_select_ui_move(DifficultySelectUI *ui, int direction);

// 선택된 난이도 가져오기
AIDifficulty difficulty_select_ui_get_selected(const DifficultySelectUI *ui);

// 규칙 선택 UI 초기화
void rule_select_ui_init(RuleSelectUI *ui);

// 규칙 선택 UI 렌더링
void rule_select_ui_render(WINDOW *win, RuleSelectUI *ui);

// 규칙 선택 이동 (좌우/상하 지원)
void rule_select_ui_move(RuleSelectUI *ui, int direction);

// 선택된 규칙 가져오기
GameRule rule_select_ui_get_selected(const RuleSelectUI *ui);

// 난이도 선택 화면 실행 (선택된 난이도 반환, -1이면 취소)
int singleplay_select_difficulty(void);

// 규칙 선택 화면 실행 (선택된 규칙 반환, -1이면 취소)
int singleplay_select_rule(void);

// 싱글플레이 전체 설정 화면 실행 (난이도 + 규칙 선택)
// 반환: 0 = 성공 (difficulty, rule에 값 저장), -1 = 취소
int singleplay_run_settings(AIDifficulty *difficulty, GameRule *rule);

#endif // SINGLEPLAY_MENU_UI_H
