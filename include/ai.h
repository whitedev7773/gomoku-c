#ifndef AI_H
#define AI_H

#include "board.h"
#include "config.h"

typedef struct {
    ai_level_t level;
} ai_context_t;

void ai_init(ai_context_t *ctx, ai_level_t level);
move_t ai_choose_move(ai_context_t *ctx, const board_t *board, stone_t stone);

#endif // AI_H
