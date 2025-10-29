#ifndef BOARD_H
#define BOARD_H

#define BOARD_SIZE 15

extern int board[BOARD_SIZE][BOARD_SIZE];

void board_init(void);
int board_place(int x, int y, int stone);

#endif
