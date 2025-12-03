#include "board_ui.h"
#include <string.h>

// Column labels (A-T, excluding I)
static const char COL_LABELS[] = "ABCDEFGHJKLMNOPQRST";

void board_ui_init_cursor(BoardCursor *cursor) {
    if (!cursor) return;
    cursor->cursor_row = 9;
    cursor->cursor_col = 9;
}

char board_ui_col_to_char(int col) {
    if (col < 0 || col >= BOARD_SIZE) return '?';
    return COL_LABELS[col];
}

int board_ui_char_to_col(char c) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (COL_LABELS[i] == c) return i;
    }
    return -1;
}

void board_ui_draw_border(WINDOW *win) {
    wattron(win, A_BOLD);

    // Top border
    mvwprintw(win, 0, 0, "┏");
    for (int i = 1; i < 49; i++) {
        wprintw(win, "━");
    }
    wprintw(win, "┓");

    // Top coordinates
    mvwprintw(win, 1, 4, "     ");
    for (int i = 0; i < BOARD_SIZE; i++) {
        wprintw(win, "%c ", COL_LABELS[i]);
    }

    // Board frame top
    mvwprintw(win, 2, 4, "   ┌");
    for (int i = 0; i < 39; i++) {
        wprintw(win, "─");
    }
    wprintw(win, "┐");

    // Side borders and row numbers
    for (int row = 0; row < BOARD_SIZE; row++) {
        int y = 3 + row;
        mvwprintw(win, y, 0, "┃");
        mvwprintw(win, y, 1, "%2d│", BOARD_SIZE - row);
        mvwprintw(win, y, 46, "│%-2d", BOARD_SIZE - row);
        mvwprintw(win, y, 49, "┃");
    }

    // Board frame bottom
    mvwprintw(win, 22, 4, "   └");
    for (int i = 0; i < 39; i++) {
        wprintw(win, "─");
    }
    wprintw(win, "┘");

    // Bottom coordinates
    mvwprintw(win, 23, 4, "     ");
    for (int i = 0; i < BOARD_SIZE; i++) {
        wprintw(win, "%c ", COL_LABELS[i]);
    }

    // Bottom border
    mvwprintw(win, 24, 0, "┣");
    for (int i = 1; i < 17; i++) wprintw(win, "━");
    wprintw(win, "┳");
    for (int i = 19; i < 32; i++) wprintw(win, "━");
    wprintw(win, "┳");
    for (int i = 34; i < 49; i++) wprintw(win, "━");
    wprintw(win, "┻");

    mvwprintw(win, 25, 0, "┃");
    mvwprintw(win, 25, 17, "┃");
    mvwprintw(win, 25, 32, "┃");
    mvwprintw(win, 25, 49, "┃");

    wattroff(win, A_BOLD);
}

void board_ui_draw_board(WINDOW *win, const Board *board, const BoardCursor *cursor) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        int y = 3 + row;
        int x = 5;

        wmove(win, y, x);

        for (int col = 0; col < BOARD_SIZE; col++) {
            Stone stone = board_get_stone(board, row, col);

            bool is_cursor = (cursor && cursor->cursor_row == row && cursor->cursor_col == col);
            bool is_last_move = (board->last_row == row && board->last_col == col);

            if (is_cursor) {
                wattron(win, A_REVERSE);
            }

            if (stone == BLACK) {
                if (is_last_move) wattron(win, COLOR_PAIR(3));
                wprintw(win, "●");
                if (is_last_move) wattroff(win, COLOR_PAIR(3));
            } else if (stone == WHITE) {
                if (is_last_move) wattron(win, COLOR_PAIR(4));
                wprintw(win, "○");
                if (is_last_move) wattroff(win, COLOR_PAIR(4));
            } else {
                wprintw(win, "·");
            }

            if (is_cursor) {
                wattroff(win, A_REVERSE);
            }

            if (col < BOARD_SIZE - 1) {
                waddch(win, ' ');
            }
        }
    }
}

void board_ui_render(WINDOW *win, const Board *board, const BoardCursor *cursor) {
    if (!win) return;

    wclear(win);
    board_ui_draw_border(win);

    if (board) {
        board_ui_draw_board(win, board, cursor);
    }

    wrefresh(win);
}

void board_ui_move_cursor(BoardCursor *cursor, int dr, int dc) {
    if (!cursor) return;

    int new_row = cursor->cursor_row + dr;
    int new_col = cursor->cursor_col + dc;

    if (new_row >= 0 && new_row < BOARD_SIZE) {
        cursor->cursor_row = new_row;
    }

    if (new_col >= 0 && new_col < BOARD_SIZE) {
        cursor->cursor_col = new_col;
    }
}
