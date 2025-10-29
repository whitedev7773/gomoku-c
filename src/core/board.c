#include "board.h"
#include <string.h>

int board[15][15];

void board_init(void) {
    memset(board, 0, sizeof(board));
}

int board_place(int x, int y, int stone) {
    if (board[y][x] != 0) return 0;
    board[y][x] = stone;
    return 1;
}
