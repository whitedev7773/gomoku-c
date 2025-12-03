#ifndef RULES_H
#define RULES_H

#include "board.h"
#include <stdbool.h>

bool rules_is_forbidden_move(const Board *board, int row, int col, Stone stone, GameRule rule, ForbiddenType *forbidden_type);

bool rules_check_double_three(const Board *board, int row, int col, Stone stone);

bool rules_check_double_four(const Board *board, int row, int col, Stone stone);

bool rules_check_overline(const Board *board, int row, int col, Stone stone);

bool rules_is_swap_available(const Board *board);

void rules_apply_swap(Board *board);

#endif // RULES_H
