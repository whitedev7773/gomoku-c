#ifndef GAME_INFO_UI_H
#define GAME_INFO_UI_H

#include "../../game/core/board.h"
#include "../../game/core/turn_manager.h"
#include "../core/ui_manager.h"
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
} GameInfoUI;

void game_info_ui_init(GameInfoUI *ui);

// 초기 렌더링 (테두리 포함)
void game_info_ui_init_bottom(WINDOW *win);

// 최적화된 업데이트 (변경된 부분만)
void game_info_ui_update_bottom(WINDOW *win, const Board *board,
                                const TurnManager *turn_mgr,
                                GameInfoUI *ui);

void game_info_ui_draw_last_move(WINDOW *win, const Board *board);

void game_info_ui_draw_current_turn(WINDOW *win, Stone current_player);

void game_info_ui_draw_timer(WINDOW *win, int remaining_seconds);

void game_info_ui_draw_play_time(WINDOW *win, const GameInfoUI *ui);

void game_info_ui_draw_buttons(WINDOW *win);

int game_info_ui_get_elapsed_seconds(const GameInfoUI *ui);

// ============================================
// 선택적 렌더링 함수 (Dirty Flag 기반)
// ============================================

// 선택적 렌더링 (dirty flag 기반)
void game_info_ui_selective_render(WINDOW *win, const Board *board,
                                   const TurnManager *turn_mgr,
                                   GameInfoUI *ui,
                                   UIRenderFlags *flags,
                                   bool first_render);

// 상단 Info 영역 텍스트 표시 함수 (stdscr에 직접 그림)
void game_info_draw_opponent_name(const char *name); // 상대방 이름 표시
void game_info_draw_viewers(int count);              // 뷰어 수 표시
void game_info_draw_ping(int ping_ms);               // PING 표시
void game_info_draw_port(int port);                  // PORT 표시

#endif // GAME_INFO_UI_H
