#include "ai_common.h"

#include <stdlib.h>

#include "board.h"

static const int directions[4][2] = {
    {1, 0},
    {0, 1},
    {1, 1},
    {1, -1}
};

stone_t ai_common_opponent(stone_t stone) {
    return stone == STONE_BLACK ? STONE_WHITE : STONE_BLACK;
}

static int count_direction(const board_t *board, int row, int col, int drow, int dcol, stone_t stone) {
    int count = 0;
    int r = row + drow;
    int c = col + dcol;
    while (board_is_inside(r, c) && board->cells[r][c] == stone) {
        count++;
        r += drow;
        c += dcol;
    }
    return count;
}

static int count_open_end(const board_t *board, int row, int col, int drow, int dcol, stone_t stone) {
    int r = row;
    int c = col;
    do {
        r += drow;
        c += dcol;
    } while (board_is_inside(r, c) && board->cells[r][c] == stone);

    if (!board_is_inside(r, c)) {
        return 0;
    }
    return board->cells[r][c] == STONE_EMPTY ? 1 : 0;
}

bool ai_common_is_winning_move(const board_t *board, int row, int col, stone_t stone) {
    if (!board_is_inside(row, col) || !board_is_empty(board, row, col)) {
        return false;
    }
    for (int i = 0; i < 4; ++i) {
        int forward = count_direction(board, row, col, directions[i][0], directions[i][1], stone);
        int backward = count_direction(board, row, col, -directions[i][0], -directions[i][1], stone);
        int total = forward + backward + 1;
        if (total >= 5) {
            return true;
        }
    }
    return false;
}

static int score_pattern(int stones, int open_ends) {
    if (stones >= 5) {
        return 100000;
    }
    if (stones == 4 && open_ends == 2) {
        return 10000;
    }
    if (stones == 4 && open_ends == 1) {
        return 5000;
    }
    if (stones == 3 && open_ends == 2) {
        return 1500;
    }
    if (stones == 3 && open_ends == 1) {
        return 800;
    }
    if (stones == 2 && open_ends == 2) {
        return 400;
    }
    if (stones == 2 && open_ends == 1) {
        return 150;
    }
    if (stones == 1 && open_ends == 2) {
        return 50;
    }
    return 10;
}

int ai_common_score_move(const board_t *board, int row, int col, stone_t stone) {
    if (!board_is_inside(row, col) || !board_is_empty(board, row, col)) {
        return -1;
    }
    int score = 0;
    for (int i = 0; i < 4; ++i) {
        int forward = count_direction(board, row, col, directions[i][0], directions[i][1], stone);
        int backward = count_direction(board, row, col, -directions[i][0], -directions[i][1], stone);
        int open = count_open_end(board, row, col, directions[i][0], directions[i][1], stone) +
                   count_open_end(board, row, col, -directions[i][0], -directions[i][1], stone);
        int stones = forward + backward + 1;
        score += score_pattern(stones, open);
    }

    // Encourage central positions slightly
    int center = BOARD_SIZE / 2;
    int dist = abs(row - center) + abs(col - center);
    score += (BOARD_SIZE - dist);

    return score;
}
