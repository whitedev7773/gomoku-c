#include "board.h"

#include <string.h>

void board_init(board_t *board) {
    if (!board) {
        return;
    }
    memset(board->cells, 0, sizeof(board->cells));
    memset(board->history, 0, sizeof(board->history));
    board->move_count = 0;
    board->last_move.row = -1;
    board->last_move.col = -1;
    board->last_move.stone = STONE_EMPTY;
}

bool board_is_inside(int row, int col) {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

bool board_is_empty(const board_t *board, int row, int col) {
    if (!board || !board_is_inside(row, col)) {
        return false;
    }
    return board->cells[row][col] == STONE_EMPTY;
}

bool board_place_stone(board_t *board, int row, int col, stone_t stone) {
    if (!board || !board_is_inside(row, col) || stone == STONE_EMPTY) {
        return false;
    }
    if (!board_is_empty(board, row, col)) {
        return false;
    }
    board->cells[row][col] = stone;
    if (board->move_count < MAX_MOVES) {
        board->history[board->move_count].row = row;
        board->history[board->move_count].col = col;
        board->history[board->move_count].stone = stone;
        board->move_count++;
    }
    board->last_move.row = row;
    board->last_move.col = col;
    board->last_move.stone = stone;
    return true;
}

bool board_undo_last(board_t *board, move_t *undone_move) {
    if (!board || board->move_count <= 0) {
        return false;
    }
    board->move_count--;
    move_t move = board->history[board->move_count];
    if (board_is_inside(move.row, move.col)) {
        board->cells[move.row][move.col] = STONE_EMPTY;
    }
    if (undone_move) {
        *undone_move = move;
    }
    if (board->move_count > 0) {
        board->last_move = board->history[board->move_count - 1];
    } else {
        board->last_move.row = -1;
        board->last_move.col = -1;
        board->last_move.stone = STONE_EMPTY;
    }
    return true;
}

bool board_peek_last(const board_t *board, move_t *move) {
    if (!board || board->move_count <= 0) {
        return false;
    }
    if (move) {
        *move = board->history[board->move_count - 1];
    }
    return true;
}

static int board_count_direction(const board_t *board, int start_row, int start_col, int drow, int dcol, stone_t stone) {
    int count = 0;
    int row = start_row;
    int col = start_col;
    while (board_is_inside(row, col) && board->cells[row][col] == stone) {
        count++;
        row += drow;
        col += dcol;
    }
    return count;
}

bool board_check_win(const board_t *board, const move_t *last_move, int target) {
    if (!board || !last_move || last_move->stone == STONE_EMPTY) {
        return false;
    }
    static const int directions[4][2] = {
        {1, 0},
        {0, 1},
        {1, 1},
        {1, -1}
    };
    for (int i = 0; i < 4; ++i) {
        int forward = board_count_direction(board, last_move->row, last_move->col, directions[i][0], directions[i][1], last_move->stone);
        int backward = board_count_direction(board, last_move->row, last_move->col, -directions[i][0], -directions[i][1], last_move->stone);
        int total = forward + backward - 1;
        if (total >= target) {
            return true;
        }
    }
    return false;
}

int board_count_consecutive(const board_t *board, int start_row, int start_col, int drow, int dcol, stone_t stone) {
    if (!board || stone == STONE_EMPTY) {
        return 0;
    }
    return board_count_direction(board, start_row, start_col, drow, dcol, stone);
}
