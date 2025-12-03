#ifndef GAME_INFO_UI_H
#define GAME_INFO_UI_H

#include "../game/board.h"
#include "../game/turn_manager.h"
#include <ncurses.h>
#include <time.h>

typedef struct {
    time_t game_start_time;
} GameInfoUI;

void game_info_ui_init(GameInfoUI *ui);

void game_info_ui_render_bottom(WINDOW *win, const Board *board,
                                 const TurnManager *turn_mgr,
                                 const GameInfoUI *ui);

void game_info_ui_draw_last_move(WINDOW *win, const Board *board);

void game_info_ui_draw_current_turn(WINDOW *win, Stone current_player);

void game_info_ui_draw_timer(WINDOW *win, int remaining_seconds);

void game_info_ui_draw_play_time(WINDOW *win, const GameInfoUI *ui);

void game_info_ui_draw_buttons(WINDOW *win);

int game_info_ui_get_elapsed_seconds(const GameInfoUI *ui);

#endif // GAME_INFO_UI_H
