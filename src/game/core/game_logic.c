#include "game_logic.h"
#include <string.h>

static int count_direction(const Board *board, int row, int col, Stone stone, int dr, int dc) {
    int count = 0;
    int r = row + dr;
    int c = col + dc;

    while (board_is_valid_position(r, c) && board_get_stone(board, r, c) == stone) {
        count++;
        r += dr;
        c += dc;
    }

    return count;
}

static bool check_line(const Board *board, int row, int col, Stone stone,
                      int dr, int dc, WinningLine *line) {
    if (board_get_stone(board, row, col) != stone) {
        return false;
    }

    int count = 1;
    count += count_direction(board, row, col, stone, dr, dc);
    count += count_direction(board, row, col, stone, -dr, -dc);

    if (count >= 5) {
        if (line) {
            int forward = count_direction(board, row, col, stone, dr, dc);
            int backward = count_direction(board, row, col, stone, -dr, -dc);

            line->start_row = row - dr * backward;
            line->start_col = col - dc * backward;
            line->end_row = row + dr * forward;
            line->end_col = col + dc * forward;

            if (dr == 0) line->direction = 0;        // horizontal
            else if (dc == 0) line->direction = 1;    // vertical
            else if (dr == dc) line->direction = 2;   // diagonal
            else line->direction = 3;                 // anti-diagonal
        }
        return true;
    }

    return false;
}

bool game_has_five_in_row(const Board *board, int row, int col, Stone stone, WinningLine *line) {
    if (!board || stone == EMPTY) {
        return false;
    }

    if (!board_is_valid_position(row, col)) {
        return false;
    }

    // Check horizontal
    if (check_line(board, row, col, stone, 0, 1, line)) {
        return true;
    }

    // Check vertical
    if (check_line(board, row, col, stone, 1, 0, line)) {
        return true;
    }

    // Check diagonal (top-left to bottom-right)
    if (check_line(board, row, col, stone, 1, 1, line)) {
        return true;
    }

    // Check anti-diagonal (top-right to bottom-left)
    if (check_line(board, row, col, stone, 1, -1, line)) {
        return true;
    }

    return false;
}

GameResult game_check_winner(const Board *board) {
    if (!board) {
        return GAME_ONGOING;
    }

    Position last_move = board_get_last_move(board);

    if (last_move.row == -1 || last_move.col == -1) {
        return GAME_ONGOING;
    }

    Stone last_stone = board_get_stone(board, last_move.row, last_move.col);

    if (last_stone == EMPTY) {
        return GAME_ONGOING;
    }

    if (game_has_five_in_row(board, last_move.row, last_move.col, last_stone, NULL)) {
        return (last_stone == BLACK) ? GAME_BLACK_WIN : GAME_WHITE_WIN;
    }

    if (game_is_draw(board)) {
        return GAME_DRAW;
    }

    return GAME_ONGOING;
}

bool game_is_draw(const Board *board) {
    if (!board) return false;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board_is_empty(board, i, j)) {
                return false;
            }
        }
    }

    return true;
}

bool game_is_over(const Board *board) {
    GameResult result = game_check_winner(board);
    return (result != GAME_ONGOING);
}
