#ifndef BOARD_UI_H
#define BOARD_UI_H

#include "../game/board.h"
#include <ncurses.h>

typedef struct {
    int cursor_row;
    int cursor_col;
} BoardCursor;

void board_ui_init_cursor(BoardCursor *cursor);

void board_ui_render(WINDOW *win, const Board *board, const BoardCursor *cursor);

void board_ui_draw_border(WINDOW *win);

void board_ui_draw_coordinates(WINDOW *win);

void board_ui_draw_board(WINDOW *win, const Board *board, const BoardCursor *cursor);

void board_ui_move_cursor(BoardCursor *cursor, int dr, int dc);

char board_ui_col_to_char(int col);

int board_ui_char_to_col(char c);

#endif // BOARD_UI_H
