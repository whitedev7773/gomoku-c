#ifndef INFO_BOTTOM_UI_H
#define INFO_BOTTOM_UI_H

#include "../../../../game/core/board.h"
#include "../../../../game/core/turn_manager.h"
#include "../../../core/ui_manager.h"
#include "last_move_display.h"
#include "now_turn_display.h"
#include "time_display.h"
#include "system_log_display.h"
#include <ncurses.h>
#include <time.h>

// 하단 UI 통합 관리 구조체
typedef struct
{
    LastMoveDisplay last_move;
    NowTurnDisplay now_turn;
    TimeDisplay time_display;
    SystemLogDisplay system_log;
} InfoBottomUI;

void info_btm_init(InfoBottomUI *ui);

// 초기 렌더링 (테두리 포함)
void info_btm_init_win(WINDOW *win);

// 최적화된 업데이트 (변경된 부분만)
void info_btm_update(WINDOW *win, const Board *board,
                     const TurnManager *turn_mgr,
                     InfoBottomUI *ui);

// 버튼 그리기
void info_btm_draw_btns(WINDOW *win);

// ============================================
// 선택적 렌더링 함수 (Dirty Flag 기반)
// ============================================

// 선택적 렌더링 (dirty flag 기반)
void info_btm_render(WINDOW *win, const Board *board,
                     const TurnManager *turn_mgr,
                     InfoBottomUI *ui,
                     UIRenderFlags *flags,
                     bool first_render);

// 시스템 로그 추가 (편의 함수)
void info_btm_add_system_log(InfoBottomUI *ui, const char *message);

// ============================================
// 하위 호환성을 위한 함수 (deprecated, 분리된 모듈 사용 권장)
// ============================================

// 아래 함수들은 분리된 모듈로 이동됨:
// - last_move_display_draw() -> info_btm_draw_last_mv() 대체
// - now_turn_display_draw() -> info_btm_draw_turn() 대체
// - time_display_draw_timer() -> info_btm_draw_timer() 대체
// - time_display_draw_playtime() -> info_btm_draw_playtime() 대체
// - time_display_elapsed_sec() -> info_btm_elapsed_sec() 대체

void info_btm_draw_last_mv(WINDOW *win, const Board *board);
void info_btm_draw_turn(WINDOW *win, Stone current_player);
void info_btm_draw_timer(WINDOW *win, int remaining_seconds);
void info_btm_draw_playtime(WINDOW *win, const TimeDisplay *time_display);
int info_btm_elapsed_sec(const TimeDisplay *time_display);

#endif // INFO_BOTTOM_UI_H
