#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "board.h"
#include <stdbool.h>

typedef enum {
    GAME_ONGOING = 0,
    GAME_BLACK_WIN = 1,
    GAME_WHITE_WIN = 2,
    GAME_DRAW = 3
} GameResult;

typedef struct {
    int start_row;
    int start_col;
    int end_row;
    int end_col;
    int direction; // 0: horizontal, 1: vertical, 2: diagonal, 3: anti-diagonal
} WinningLine;

GameResult game_check_winner(const Board *board);

bool game_has_five_in_row(const Board *board, int row, int col, Stone stone, WinningLine *line);

bool game_is_over(const Board *board);

bool game_is_draw(const Board *board);

#endif // GAME_LOGIC_H
