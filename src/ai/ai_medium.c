#include "ai_common.h"

#include <limits.h>
#include <stdlib.h>

move_t ai_medium_choose(const board_t *board, stone_t stone) {
    move_t move = {-1, -1, stone};
    if (!board) {
        return move;
    }

    // 1. Check for immediate winning move
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            if (board_is_empty(board, row, col) &&
                ai_common_is_winning_move(board, row, col, stone)) {
                move.row = row;
                move.col = col;
                return move;
            }
        }
    }

    stone_t opponent = ai_common_opponent(stone);

    // 2. Block opponent's immediate win
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            if (board_is_empty(board, row, col) &&
                ai_common_is_winning_move(board, row, col, opponent)) {
                move.row = row;
                move.col = col;
                return move;
            }
        }
    }

    // 3. Choose based on heuristic score
    int best_score = INT_MIN;
    int center = BOARD_SIZE / 2;
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            if (!board_is_empty(board, row, col)) {
                continue;
            }
            int score = ai_common_score_move(board, row, col, stone);
            int block_score = ai_common_score_move(board, row, col, opponent);
            score += block_score / 2; // slightly prefer blocking strong spots
            if (score > best_score) {
                best_score = score;
                move.row = row;
                move.col = col;
            } else if (score == best_score) {
                int dist_current = abs(row - center) + abs(col - center);
                int dist_best = abs(move.row - center) + abs(move.col - center);
                if (dist_current < dist_best) {
                    move.row = row;
                    move.col = col;
                }
            }
        }
    }

    if (move.row == -1) {
        move.row = center;
        move.col = center;
    }
    return move;
}
