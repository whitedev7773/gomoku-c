#include <ncurses.h>

#include "game.h"

static char stone_symbol(stone_t stone) {
    switch (stone) {
        case STONE_BLACK:
            return 'X';
        case STONE_WHITE:
            return 'O';
        default:
            return '.';
    }
}

void board_view_render(WINDOW *win, const game_t *game, int cursor_row, int cursor_col) {
    if (!win || !game) {
        return;
    }
    werase(win);
    box(win, 0, 0);

    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            int y = row + 1;
            int x = col * 2 + 2;
            stone_t stone = game->board.cells[row][col];
            bool is_cursor = (row == cursor_row && col == cursor_col);
            bool is_last = (game->last_move.row == row && game->last_move.col == col);
            bool is_prev = (game->previous_move.row == row && game->previous_move.col == col);

            if (is_cursor) {
                wattron(win, A_REVERSE);
            }
            if (is_last) {
                wattron(win, A_BOLD);
            } else if (is_prev) {
                wattron(win, A_DIM);
            }

            mvwaddch(win, y, x, stone_symbol(stone));

            if (is_last) {
                wattroff(win, A_BOLD);
            } else if (is_prev) {
                wattroff(win, A_DIM);
            }
            if (is_cursor) {
                wattroff(win, A_REVERSE);
            }
        }
    }

    mvwprintw(win, BOARD_SIZE + 1, 2, "Arrows: move  Enter: place  Tab: chat");
    wrefresh(win);
}
