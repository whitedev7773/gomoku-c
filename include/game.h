#ifndef GAME_H
#define GAME_H

#include "board.h"
#include "tui.h"
#include "network.h"
#include "ai.h"

void game_loop(void);
void game_init(void);
void game_cleanup(void);

#endif
