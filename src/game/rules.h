#ifndef RULES_H
#define RULES_H

#include "board.h"
#include <stdbool.h>

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

bool rules_is_forbidden_move(const Board *board, int row, int col, Stone stone, GameRule rule, ForbiddenType *forbidden_type);

bool rules_check_double_three(const Board *board, int row, int col, Stone stone);

bool rules_check_double_four(const Board *board, int row, int col, Stone stone);

bool rules_check_overline(const Board *board, int row, int col, Stone stone);

bool rules_is_swap_available(const Board *board);

void rules_apply_swap(Board *board);

#endif // RULES_H
