#include "board.h"
#include "rules.h"
#include <string.h>

void board_init(Board *board) {
    if (!board) return;

    memset(board->cells, EMPTY, sizeof(board->cells));
    board->move_count = 0;
    board->last_row = -1;
    board->last_col = -1;
    board->rule = RULE_STANDARD;
    memset(board->forbidden_marks, 0, sizeof(board->forbidden_marks));
}

void board_init_with_rule(Board *board, GameRule rule) {
    board_init(board);
    if (board) {
        board->rule = rule;
    }
}

bool board_is_valid_position(int row, int col) {
    return (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE);
}

bool board_place_stone(Board *board, int row, int col, Stone stone) {
    if (!board) return false;

    if (!board_is_valid_position(row, col)) {
        return false;
    }

    if (board->cells[row][col] != EMPTY) {
        return false;
    }

    if (stone != BLACK && stone != WHITE) {
        return false;
    }

    board->cells[row][col] = stone;
    board->last_row = row;
    board->last_col = col;
    board->move_count++;

    return true;
}

Stone board_get_stone(const Board *board, int row, int col) {
    if (!board || !board_is_valid_position(row, col)) {
        return EMPTY;
    }

    return board->cells[row][col];
}

bool board_is_empty(const Board *board, int row, int col) {
    return board_get_stone(board, row, col) == EMPTY;
}

void board_clear(Board *board) {
    board_init(board);
}

Position board_get_last_move(const Board *board) {
    Position pos = {-1, -1};

    if (!board) return pos;

    pos.row = board->last_row;
    pos.col = board->last_col;

    return pos;
}

int board_get_move_count(const Board *board) {
    if (!board) return 0;
    return board->move_count;
}

void board_set_rule(Board *board, GameRule rule) {
    if (!board) return;
    board->rule = rule;
}

GameRule board_get_rule(const Board *board) {
    if (!board) return RULE_STANDARD;
    return board->rule;
}

void board_update_forbidden_marks(Board *board, Stone current_player) {
    if (!board || board->rule != RULE_RENJU || current_player != BLACK) {
        // Renju Rule은 흑돌에만 적용
        memset(board->forbidden_marks, 0, sizeof(board->forbidden_marks));
        return;
    }

    // 모든 빈 자리에 대해 금수 체크
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            if (board->cells[row][col] == EMPTY) {
                ForbiddenType forbidden_type;
                board->forbidden_marks[row][col] =
                    rules_is_forbidden_move(board, row, col, BLACK, board->rule, &forbidden_type);
            } else {
                board->forbidden_marks[row][col] = false;
            }
        }
    }
}

bool board_is_forbidden(const Board *board, int row, int col) {
    if (!board || !board_is_valid_position(row, col)) {
        return false;
    }
    return board->forbidden_marks[row][col];
}
