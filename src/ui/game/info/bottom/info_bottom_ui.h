#ifndef INFO_BOTTOM_UI_H
#define INFO_BOTTOM_UI_H

#include "../../../../game/core/board.h"
#include "../../../../game/core/turn_manager.h"
#include "../../../core/ui_manager.h"
#include <ncurses.h>
#include <time.h>

typedef struct
{
    time_t game_start_time;
    // 최적화를 위한 이전 상태 추적
    int prev_last_row;
    int prev_last_col;
    Stone prev_current_player;
    int prev_elapsed_seconds;
    int prev_remaining_seconds; // 타이머 이전 값
} InfoBottomUI;

void info_btm_init(InfoBottomUI *ui);

// 초기 렌더링 (테두리 포함)
void info_btm_init_win(WINDOW *win);

// 최적화된 업데이트 (변경된 부분만)
void info_btm_update(WINDOW *win, const Board *board,
                           const TurnManager *turn_mgr,
                           InfoBottomUI *ui);

void info_btm_draw_last_mv(WINDOW *win, const Board *board);

void info_btm_draw_turn(WINDOW *win, Stone current_player);

void info_btm_draw_timer(WINDOW *win, int remaining_seconds);

void info_btm_draw_playtime(WINDOW *win, const InfoBottomUI *ui);

void info_btm_draw_btns(WINDOW *win);

int info_btm_elapsed_sec(const InfoBottomUI *ui);

// ============================================
// 선택적 렌더링 함수 (Dirty Flag 기반)
// ============================================

// 선택적 렌더링 (dirty flag 기반)
void info_btm_render(WINDOW *win, const Board *board,
                                     const TurnManager *turn_mgr,
                                     InfoBottomUI *ui,
                                     UIRenderFlags *flags,
                                     bool first_render);

#endif // INFO_BOTTOM_UI_H
