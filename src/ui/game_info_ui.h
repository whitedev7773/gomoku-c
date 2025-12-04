#ifndef GAME_INFO_UI_H
#define GAME_INFO_UI_H

#include "../game/board.h"
#include "../game/turn_manager.h"
#include <ncurses.h>
#include <time.h>

typedef struct {
    time_t game_start_time;
    // 최적화를 위한 이전 상태 추적
    int prev_last_row;
    int prev_last_col;
    Stone prev_current_player;
    int prev_elapsed_seconds;
} GameInfoUI;

void game_info_ui_init(GameInfoUI *ui);

// 초기 렌더링 (테두리 포함)
void game_info_ui_init_bottom(WINDOW *win);

// 전체 렌더링 (호환성)
void game_info_ui_render_bottom(WINDOW *win, const Board *board,
                                 const TurnManager *turn_mgr,
                                 const GameInfoUI *ui);

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

#endif // GAME_INFO_UI_H
