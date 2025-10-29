#include <stdio.h>
#include <ncurses.h>
#include "game.h"

int main(void) {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);

    game_init();
    game_loop();
    game_cleanup();

    endwin();
    return 0;
}
