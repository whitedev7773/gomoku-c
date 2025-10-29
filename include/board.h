#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>

#define BOARD_SIZE 15
#define MAX_MOVES (BOARD_SIZE * BOARD_SIZE)

typedef enum {
    STONE_EMPTY = 0,
    STONE_BLACK = 1,
    STONE_WHITE = 2
} stone_t;

typedef struct {
    int row;
    int col;
    stone_t stone;
} move_t;

typedef struct {
    stone_t cells[BOARD_SIZE][BOARD_SIZE];
    move_t history[MAX_MOVES];
    int move_count;
    move_t last_move;
} board_t;

void board_init(board_t *board);
bool board_is_inside(int row, int col);
bool board_is_empty(const board_t *board, int row, int col);
bool board_place_stone(board_t *board, int row, int col, stone_t stone);
bool board_undo_last(board_t *board, move_t *undone_move);
bool board_peek_last(const board_t *board, move_t *move);
bool board_check_win(const board_t *board, const move_t *last_move, int target);
int board_count_consecutive(const board_t *board, int start_row, int start_col, int drow, int dcol, stone_t stone);

#endif // BOARD_H
