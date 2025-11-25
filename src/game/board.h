#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>

#define BOARD_SIZE 19

typedef enum {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
} Stone;

typedef struct {
    Stone cells[BOARD_SIZE][BOARD_SIZE];
    int move_count;
    int last_row;
    int last_col;
} Board;

typedef struct {
    int row;
    int col;
} Position;

void board_init(Board *board);

bool board_is_valid_position(int row, int col);

bool board_place_stone(Board *board, int row, int col, Stone stone);

Stone board_get_stone(const Board *board, int row, int col);

bool board_is_empty(const Board *board, int row, int col);

void board_clear(Board *board);

Position board_get_last_move(const Board *board);

int board_get_move_count(const Board *board);

#endif // BOARD_H
