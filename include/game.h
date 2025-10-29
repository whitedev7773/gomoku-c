#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#include "ai.h"
#include "board.h"
#include "config.h"
#include "timer.h"

typedef enum {
    GAME_MODE_SINGLE = 0,
    GAME_MODE_MULTI_HOST,
    GAME_MODE_MULTI_CLIENT
} game_mode_t;

typedef enum {
    PLAYER_LOCAL = 0,
    PLAYER_AI,
    PLAYER_REMOTE
} player_kind_t;

typedef enum {
    GAME_STATUS_INIT = 0,
    GAME_STATUS_RUNNING,
    GAME_STATUS_FINISHED
} game_status_t;

typedef struct {
    board_t board;
    game_mode_t mode;
    player_kind_t players[2];
    char nicknames[2][CONFIG_MAX_NICKNAME];
    ai_context_t ai;
    int current_player;
    int winner; // -1 none, 0/1 winner index
    bool finished;
    move_t last_move;
    move_t previous_move;
    bool undo_pending;
    int undo_requester;
    turn_timer_t timers[2];
    int turn_time_limit;
    game_status_t status;
} game_t;

stone_t game_player_stone(int player_index);
void game_init_single(game_t *game, const config_t *cfg, ai_level_t level);
void game_init_multi_host(game_t *game, const config_t *cfg);
void game_init_multi_client(game_t *game, const config_t *cfg);

bool game_can_place(const game_t *game, int row, int col);
bool game_place_stone(game_t *game, int row, int col);
bool game_ai_turn(game_t *game, move_t *out_move);

void game_start(game_t *game);
void game_finish(game_t *game, int winner_index);
void game_switch_turn(game_t *game);
int game_current_player(const game_t *game);
stone_t game_current_stone(const game_t *game);
bool game_is_local_turn(const game_t *game);

void game_request_undo(game_t *game, int requester);
bool game_apply_undo(game_t *game);
void game_cancel_undo(game_t *game);

void game_resign(game_t *game, int player_index);

void game_reset_timer(game_t *game);
int game_timer_remaining(const game_t *game, int player_index);

#endif // GAME_H
