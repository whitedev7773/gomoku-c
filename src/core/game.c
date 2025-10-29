#include "game.h"

#include <stdio.h>
#include <string.h>

#include "board.h"
#include "timer.h"
#include "utils.h"

static void game_init_common(game_t *game, const config_t *cfg) {
    memset(game, 0, sizeof(*game));
    board_init(&game->board);
    game->mode = GAME_MODE_SINGLE;
    game->players[0] = PLAYER_LOCAL;
    game->players[1] = PLAYER_REMOTE;
    snprintf(game->nicknames[0], sizeof(game->nicknames[0]), "%s", cfg ? cfg->nickname : "Player");
    snprintf(game->nicknames[1], sizeof(game->nicknames[1]), "Opponent");
    game->current_player = 0;
    game->winner = -1;
    game->finished = false;
    game->undo_pending = false;
    game->undo_requester = -1;
    game->turn_time_limit = cfg ? cfg->turn_time_seconds : 30;
    game->status = GAME_STATUS_INIT;
    game->last_move.row = -1;
    game->previous_move.row = -1;
    timer_stop(&game->timers[0]);
    timer_stop(&game->timers[1]);
}

stone_t game_player_stone(int player_index) {
    return player_index == 0 ? STONE_BLACK : STONE_WHITE;
}

void game_init_single(game_t *game, const config_t *cfg, ai_level_t level) {
    if (!game) {
        return;
    }
    game_init_common(game, cfg);
    game->mode = GAME_MODE_SINGLE;
    game->players[0] = PLAYER_LOCAL;
    game->players[1] = PLAYER_AI;
    snprintf(game->nicknames[1], sizeof(game->nicknames[1]), "AI (%s)", config_ai_level_name(level));
    ai_init(&game->ai, level);
}

void game_init_multi_host(game_t *game, const config_t *cfg) {
    if (!game) {
        return;
    }
    game_init_common(game, cfg);
    game->mode = GAME_MODE_MULTI_HOST;
    game->players[0] = PLAYER_LOCAL;
    game->players[1] = PLAYER_REMOTE;
    snprintf(game->nicknames[1], sizeof(game->nicknames[1]), "Remote");
}

void game_init_multi_client(game_t *game, const config_t *cfg) {
    if (!game) {
        return;
    }
    game_init_common(game, cfg);
    game->mode = GAME_MODE_MULTI_CLIENT;
    game->players[0] = PLAYER_REMOTE;
    game->players[1] = PLAYER_LOCAL;
    snprintf(game->nicknames[0], sizeof(game->nicknames[0]), "Remote");
    snprintf(game->nicknames[1], sizeof(game->nicknames[1]), "%s", cfg ? cfg->nickname : "Player");
}

static bool game_player_is_local(const game_t *game, int player_index) {
    if (!game || player_index < 0 || player_index > 1) {
        return false;
    }
    return game->players[player_index] == PLAYER_LOCAL;
}

void game_start(game_t *game) {
    if (!game) {
        return;
    }
    game->status = GAME_STATUS_RUNNING;
    game->current_player = 0;
    game_reset_timer(game);
}

void game_finish(game_t *game, int winner_index) {
    if (!game) {
        return;
    }
    game->winner = winner_index;
    game->finished = true;
    game->status = GAME_STATUS_FINISHED;
    timer_stop(&game->timers[0]);
    timer_stop(&game->timers[1]);
}

int game_current_player(const game_t *game) {
    return game ? game->current_player : 0;
}

stone_t game_current_stone(const game_t *game) {
    return game_player_stone(game_current_player(game));
}

bool game_is_local_turn(const game_t *game) {
    if (!game) {
        return false;
    }
    return game_player_is_local(game, game->current_player);
}

bool game_can_place(const game_t *game, int row, int col) {
    if (!game || game->status != GAME_STATUS_RUNNING || game->finished) {
        return false;
    }
    return board_is_empty(&game->board, row, col);
}

static void game_update_last_moves(game_t *game, const move_t *move) {
    if (!game || !move) {
        return;
    }
    game->previous_move = game->last_move;
    game->last_move = *move;
}

bool game_place_stone(game_t *game, int row, int col) {
    if (!game) {
        return false;
    }
    if (!game_can_place(game, row, col)) {
        return false;
    }
    stone_t stone = game_current_stone(game);
    if (!board_place_stone(&game->board, row, col, stone)) {
        return false;
    }

    move_t placed = {row, col, stone};
    game_update_last_moves(game, &placed);

    if (board_check_win(&game->board, &placed, 5)) {
        game_finish(game, game->current_player);
        return true;
    }

    if (game->board.move_count >= MAX_MOVES) {
        game_finish(game, -1);
        return true;
    }

    game_switch_turn(game);
    return true;
}

bool game_ai_turn(game_t *game, move_t *out_move) {
    if (!game || game->players[game->current_player] != PLAYER_AI) {
        return false;
    }
    move_t move = ai_choose_move(&game->ai, &game->board, game_current_stone(game));
    if (!board_is_inside(move.row, move.col) || !board_is_empty(&game->board, move.row, move.col)) {
        // Fallback to first empty
        for (int r = 0; r < BOARD_SIZE; ++r) {
            for (int c = 0; c < BOARD_SIZE; ++c) {
                if (board_is_empty(&game->board, r, c)) {
                    move.row = r;
                    move.col = c;
                    move.stone = game_current_stone(game);
                    goto fallback_done;
                }
            }
        }
fallback_done:;
    }
    if (!board_place_stone(&game->board, move.row, move.col, move.stone)) {
        return false;
    }
    game_update_last_moves(game, &move);
    if (board_check_win(&game->board, &move, 5)) {
        game_finish(game, game->current_player);
    } else if (game->board.move_count >= MAX_MOVES) {
        game_finish(game, -1);
    } else {
        game_switch_turn(game);
    }
    if (out_move) {
        *out_move = move;
    }
    return true;
}

void game_switch_turn(game_t *game) {
    if (!game) {
        return;
    }
    timer_stop(&game->timers[game->current_player]);
    game->current_player = (game->current_player + 1) % 2;
    game_reset_timer(game);
}

void game_resign(game_t *game, int player_index) {
    if (!game || game->finished) {
        return;
    }
    int winner = (player_index + 1) % 2;
    game_finish(game, winner);
}

void game_request_undo(game_t *game, int requester) {
    if (!game) {
        return;
    }
    game->undo_pending = true;
    game->undo_requester = requester;
}

bool game_apply_undo(game_t *game) {
    if (!game) {
        return false;
    }
    move_t undone;
    if (!board_undo_last(&game->board, &undone)) {
        return false;
    }
    game->current_player = (undone.stone == STONE_BLACK) ? 0 : 1;
    move_t peek;
    if (board_peek_last(&game->board, &peek)) {
        game->last_move = peek;
    } else {
        game->last_move.row = -1;
        game->previous_move.row = -1;
    }
    game->finished = false;
    game->winner = -1;
    game_reset_timer(game);
    game_cancel_undo(game);
    return true;
}

void game_cancel_undo(game_t *game) {
    if (!game) {
        return;
    }
    game->undo_pending = false;
    game->undo_requester = -1;
}

void game_reset_timer(game_t *game) {
    if (!game) {
        return;
    }
    if (game->turn_time_limit <= 0) {
        timer_stop(&game->timers[0]);
        timer_stop(&game->timers[1]);
        return;
    }
    timer_stop(&game->timers[0]);
    timer_stop(&game->timers[1]);
    timer_start(&game->timers[game->current_player], game->turn_time_limit);
}

int game_timer_remaining(const game_t *game, int player_index) {
    if (!game || player_index < 0 || player_index > 1) {
        return 0;
    }
    if (game->turn_time_limit <= 0) {
        return 0;
    }
    return timer_remaining(&game->timers[player_index]);
}
