#ifndef AI_COMMON_H
#define AI_COMMON_H

#include <stdbool.h>

#include "board.h"

stone_t ai_common_opponent(stone_t stone);
bool ai_common_is_winning_move(const board_t *board, int row, int col, stone_t stone);
int ai_common_score_move(const board_t *board, int row, int col, stone_t stone);

#endif // AI_COMMON_H
