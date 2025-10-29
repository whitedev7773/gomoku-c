#include "ai_common.h"

#include <limits.h>
#include <stdlib.h>

move_t ai_hard_choose(const board_t *board, stone_t stone) {
    move_t move = {-1, -1, stone};
    if (!board) {
        return move;
    }

    // Immediate winning move
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

    // If opponent can win, block
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

    int best_value = INT_MIN;
    int center = BOARD_SIZE / 2;
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            if (!board_is_empty(board, row, col)) {
                continue;
            }
            int self_score = ai_common_score_move(board, row, col, stone);
            if (self_score < 0) {
                continue;
            }
            board_t hypothetical = *board;
            board_place_stone(&hypothetical, row, col, stone);

            int opponent_best = 0;
            for (int r2 = 0; r2 < BOARD_SIZE; ++r2) {
                for (int c2 = 0; c2 < BOARD_SIZE; ++c2) {
                    if (board_is_empty(&hypothetical, r2, c2)) {
                        if (ai_common_is_winning_move(&hypothetical, r2, c2, opponent)) {
                            opponent_best = 90000;
                            goto found_opponent_threat;
                        }
                        int s = ai_common_score_move(&hypothetical, r2, c2, opponent);
                        if (s > opponent_best) {
                            opponent_best = s;
                        }
                    }
                }
            }
found_opponent_threat:;
            int value = self_score * 2 - opponent_best;
            if (ai_common_is_winning_move(board, row, col, opponent)) {
                value += 4000; // move blocks strong threat
            }
            int dist = abs(row - center) + abs(col - center);
            value -= dist; // prefer center
            if (value > best_value) {
                best_value = value;
                move.row = row;
                move.col = col;
            }
        }
    }

    if (move.row == -1) {
        move.row = center;
        move.col = center;
    }
    return move;
}
