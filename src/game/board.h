#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>

#define BOARD_SIZE 19

typedef enum {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
} Stone;

typedef enum {
    RULE_STANDARD = 0,
    RULE_RENJU = 1
} GameRule;

typedef enum {
    FORBIDDEN_NONE = 0,
    FORBIDDEN_DOUBLE_THREE = 1,
    FORBIDDEN_DOUBLE_FOUR = 2,
    FORBIDDEN_OVERLINE = 3
} ForbiddenType;

typedef struct {
    Stone cells[BOARD_SIZE][BOARD_SIZE];
    int move_count;
    int last_row;
    int last_col;
    GameRule rule;
    bool forbidden_marks[BOARD_SIZE][BOARD_SIZE];  // 금수 위치 표시
} Board;

typedef struct {
    int row;
    int col;
} Position;

void board_init(Board *board);

void board_init_with_rule(Board *board, GameRule rule);

bool board_is_valid_position(int row, int col);

bool board_place_stone(Board *board, int row, int col, Stone stone);

Stone board_get_stone(const Board *board, int row, int col);

bool board_is_empty(const Board *board, int row, int col);

void board_clear(Board *board);

Position board_get_last_move(const Board *board);

int board_get_move_count(const Board *board);

void board_set_rule(Board *board, GameRule rule);

GameRule board_get_rule(const Board *board);

void board_update_forbidden_marks(Board *board, Stone current_player);

bool board_is_forbidden(const Board *board, int row, int col);

#endif // BOARD_H
