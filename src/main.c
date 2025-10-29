#include <stdio.h>
#include <stdlib.h>
#include "tui.h"
#include "game.h"

int main(void) {
    tui_init();
    game_start();
    tui_cleanup();
    return 0;
}
