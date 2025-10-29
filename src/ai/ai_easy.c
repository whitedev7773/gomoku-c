#include "ai_common.h"

#include <stdlib.h>

#include "utils.h"

move_t ai_easy_choose(const board_t *board, stone_t stone) {
    move_t move = {-1, -1, stone};
    if (!board) {
        return move;
    }

    int positions[MAX_MOVES][2];
    int count = 0;
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            if (board_is_empty(board, row, col)) {
                positions[count][0] = row;
                positions[count][1] = col;
                count++;
            }
        }
    }
    if (count == 0) {
        return move;
    }
    utils_seed_random();
    int choice = utils_random_int(0, count - 1);
    move.row = positions[choice][0];
    move.col = positions[choice][1];
    move.stone = stone;
    return move;
}
