#include "tui.h"
#include <ncurses.h>

void tui_draw_board(void) {
    for (int y = 0; y < 15; y++) {
        for (int x = 0; x < 15; x++) {
            mvprintw(y + 1, x * 2 + 1, "+");
        }
    }
    refresh();
}
