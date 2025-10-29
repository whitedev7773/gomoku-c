#include "ai.h"

#include "board.h"
#include "config.h"

// Forward declarations for difficulty-specific strategies
move_t ai_easy_choose(const board_t *board, stone_t stone);
move_t ai_medium_choose(const board_t *board, stone_t stone);
move_t ai_hard_choose(const board_t *board, stone_t stone);

void ai_init(ai_context_t *ctx, ai_level_t level) {
    if (!ctx) {
        return;
    }
    ctx->level = level;
}

move_t ai_choose_move(ai_context_t *ctx, const board_t *board, stone_t stone) {
    move_t move = {-1, -1, stone};
    if (!ctx || !board) {
        return move;
    }
    switch (ctx->level) {
        case AI_LEVEL_EASY:
            move = ai_easy_choose(board, stone);
            break;
        case AI_LEVEL_MEDIUM:
            move = ai_medium_choose(board, stone);
            break;
        case AI_LEVEL_HARD:
            move = ai_hard_choose(board, stone);
            break;
        default:
            move = ai_medium_choose(board, stone);
            break;
    }
    move.stone = stone;
    return move;
}
