#ifndef TURN_MANAGER_H
#define TURN_MANAGER_H

#include "board.h"
#include <stdbool.h>
#include <time.h>

#define TURN_TIMEOUT_SECONDS 20

typedef struct {
    Stone current_player;
    time_t turn_start_time;
    int total_turns;
    bool timeout_enabled;
} TurnManager;

void turn_manager_init(TurnManager *manager, Stone first_player);

Stone turn_manager_get_current_player(const TurnManager *manager);

void turn_manager_next_turn(TurnManager *manager);

bool turn_manager_is_timeout(const TurnManager *manager);

int turn_manager_get_remaining_time(const TurnManager *manager);

void turn_manager_reset_timer(TurnManager *manager);

Stone turn_manager_get_opposite_player(Stone player);

void turn_manager_set_timeout_enabled(TurnManager *manager, bool enabled);

#endif // TURN_MANAGER_H
