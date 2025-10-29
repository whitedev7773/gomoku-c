#include "tui.h"

#include <ncurses.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "input.h"
#include "utils.h"

// Forward declarations from other tui modules
void board_view_render(WINDOW *win, const game_t *game, int cursor_row, int cursor_col);
void status_bar_render(WINDOW *win, const game_t *game, const char *info_message, bool chat_mode);
void chat_view_render(WINDOW *win, const chat_log_t *log);
void chat_input_render(WINDOW *win, const char *buffer, bool active);

#define INFO_MESSAGE_DURATION 3000

struct tui_context {
    game_t *game;
    chat_log_t *chat_log;
    network_session_t *network;

    WINDOW *status_win;
    WINDOW *board_win;
    WINDOW *chat_win;
    WINDOW *input_win;

    int cursor_row;
    int cursor_col;
    bool chat_mode;

    char chat_buffer[CHAT_MESSAGE_LEN];
    char info_message[128];
    long info_expires_at;

    pthread_mutex_t event_mutex;
    network_event_t pending_events[32];
    size_t pending_count;
};

static int tui_local_player_index(const game_t *game) {
    if (!game) {
        return 0;
    }
    if (game->players[0] == PLAYER_LOCAL) {
        return 0;
    }
    if (game->players[1] == PLAYER_LOCAL) {
        return 1;
    }
    return 0;
}

static int tui_remote_player_index(const game_t *game) {
    if (!game) {
        return 1;
    }
    if (game->players[0] == PLAYER_REMOTE) {
        return 0;
    }
    if (game->players[1] == PLAYER_REMOTE) {
        return 1;
    }
    return 1;
}

static void tui_add_chat(tui_context_t *ctx, const char *sender, const char *text) {
    if (!ctx || !ctx->chat_log || !sender || !text) {
        return;
    }
    chat_log_add(ctx->chat_log, sender, text);
}

static void tui_show_message(tui_context_t *ctx, const char *message) {
    if (!ctx || !message) {
        return;
    }
    pthread_mutex_lock(&ctx->event_mutex);
    snprintf(ctx->info_message, sizeof(ctx->info_message), "%s", message);
    ctx->info_expires_at = utils_now_ms() + INFO_MESSAGE_DURATION;
    pthread_mutex_unlock(&ctx->event_mutex);
}

static void tui_init_curses(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_WHITE, -1);
        init_pair(2, COLOR_CYAN, -1);
    }
    refresh();
}

static void tui_destroy_windows(tui_context_t *ctx) {
    if (!ctx) {
        return;
    }
    if (ctx->status_win) {
        delwin(ctx->status_win);
    }
    if (ctx->board_win) {
        delwin(ctx->board_win);
    }
    if (ctx->chat_win) {
        delwin(ctx->chat_win);
    }
    if (ctx->input_win) {
        delwin(ctx->input_win);
    }
}

static void tui_resize_layout(tui_context_t *ctx) {
    if (!ctx) {
        return;
    }
    int status_height = 3;
    int input_height = 3;
    int board_height = BOARD_SIZE + 3;
    int board_width = BOARD_SIZE * 2 + 4;
    int chat_height = LINES - status_height - input_height;
    int chat_width = COLS - board_width;
    if (chat_height < 5) {
        chat_height = 5;
    }
    if (chat_width < 20) {
        chat_width = 20;
    }
    if (ctx->status_win) {
        wresize(ctx->status_win, status_height, COLS);
        mvwin(ctx->status_win, 0, 0);
    } else {
        ctx->status_win = newwin(status_height, COLS, 0, 0);
    }
    if (ctx->board_win) {
        wresize(ctx->board_win, board_height, board_width);
        mvwin(ctx->board_win, status_height, 0);
    } else {
        ctx->board_win = newwin(board_height, board_width, status_height, 0);
    }
    if (ctx->chat_win) {
        wresize(ctx->chat_win, chat_height, chat_width);
        mvwin(ctx->chat_win, status_height, board_width);
    } else {
        ctx->chat_win = newwin(chat_height, chat_width, status_height, board_width);
    }
    if (ctx->input_win) {
        wresize(ctx->input_win, input_height, COLS);
        mvwin(ctx->input_win, LINES - input_height, 0);
    } else {
        ctx->input_win = newwin(input_height, COLS, LINES - input_height, 0);
    }
}

tui_context_t *tui_create(game_t *game, chat_log_t *chat_log) {
    tui_init_curses();
    tui_context_t *ctx = calloc(1, sizeof(tui_context_t));
    if (!ctx) {
        endwin();
        return NULL;
    }
    ctx->game = game;
    ctx->chat_log = chat_log;
    ctx->cursor_row = BOARD_SIZE / 2;
    ctx->cursor_col = BOARD_SIZE / 2;
    ctx->chat_buffer[0] = '\0';
    ctx->info_message[0] = '\0';
    ctx->info_expires_at = 0;
    pthread_mutex_init(&ctx->event_mutex, NULL);
    ctx->pending_count = 0;
    tui_resize_layout(ctx);
    input_init();
    return ctx;
}

void tui_destroy(tui_context_t *ctx) {
    if (!ctx) {
        return;
    }
    input_shutdown();
    tui_destroy_windows(ctx);
    pthread_mutex_destroy(&ctx->event_mutex);
    endwin();
    free(ctx);
}

static void tui_handle_place_stone(tui_context_t *ctx) {
    if (!ctx || !ctx->game) {
        return;
    }
    if (!game_is_local_turn(ctx->game)) {
        tui_show_message(ctx, "Not your turn");
        return;
    }
    int player_index = game_current_player(ctx->game);
    if (!game_place_stone(ctx->game, ctx->cursor_row, ctx->cursor_col)) {
        tui_show_message(ctx, "Invalid move");
        return;
    }
    char msg[64];
    snprintf(msg, sizeof(msg), "%s placed (%d, %d)", ctx->game->nicknames[player_index],
             ctx->game->last_move.row + 1, ctx->game->last_move.col + 1);
    tui_add_chat(ctx, "System", msg);
    if (ctx->network) {
        network_send_move(ctx->network, ctx->game->last_move.row, ctx->game->last_move.col);
    }
}

static void tui_handle_input_event(tui_context_t *ctx, const input_event_t *event) {
    if (!ctx || !event) {
        return;
    }
    switch (event->type) {
        case INPUT_EVENT_MOVE_CURSOR:
            ctx->cursor_row = utils_clamp(ctx->cursor_row + event->data.move.drow, 0, BOARD_SIZE - 1);
            ctx->cursor_col = utils_clamp(ctx->cursor_col + event->data.move.dcol, 0, BOARD_SIZE - 1);
            break;
        case INPUT_EVENT_PLACE_STONE:
            tui_handle_place_stone(ctx);
            break;
        case INPUT_EVENT_CHAT_APPEND:
            if (strlen(ctx->chat_buffer) + 1 < CHAT_MESSAGE_LEN) {
                size_t len = strlen(ctx->chat_buffer);
                ctx->chat_buffer[len] = event->data.chat.ch;
                ctx->chat_buffer[len + 1] = '\0';
            }
            break;
        case INPUT_EVENT_CHAT_BACKSPACE: {
            size_t len = strlen(ctx->chat_buffer);
            if (len > 0) {
                ctx->chat_buffer[len - 1] = '\0';
            }
            break;
        }
        case INPUT_EVENT_CHAT_SEND: {
            if (ctx->chat_mode) {
                if (strlen(ctx->chat_buffer) > 0) {
                    int local_index = tui_local_player_index(ctx->game);
                    tui_add_chat(ctx, ctx->game->nicknames[local_index], ctx->chat_buffer);
                    if (ctx->network) {
                        network_send_chat(ctx->network, ctx->chat_buffer);
                    }
                    ctx->chat_buffer[0] = '\0';
                }
            } else {
                tui_handle_place_stone(ctx);
            }
            break;
        }
        case INPUT_EVENT_UNDO_REQUEST:
            if (ctx->network) {
                game_request_undo(ctx->game, tui_local_player_index(ctx->game));
                network_send_undo_request(ctx->network);
                tui_show_message(ctx, "Undo request sent");
            } else {
                if (game_apply_undo(ctx->game)) {
                    tui_show_message(ctx, "Last move undone");
                } else {
                    tui_show_message(ctx, "Cannot undo");
                }
            }
            break;
        case INPUT_EVENT_UNDO_ACCEPT:
            if (ctx->network) {
                network_send_undo_response(ctx->network, true);
                tui_add_chat(ctx, "System", "Undo accepted");
            } else {
                game_apply_undo(ctx->game);
            }
            break;
        case INPUT_EVENT_UNDO_DECLINE:
            if (ctx->network) {
                network_send_undo_response(ctx->network, false);
                tui_add_chat(ctx, "System", "Undo declined");
            }
            game_cancel_undo(ctx->game);
            break;
        case INPUT_EVENT_RESIGN:
            game_resign(ctx->game, game_current_player(ctx->game));
            tui_add_chat(ctx, "System", "You resigned");
            if (ctx->network) {
                network_send_resign(ctx->network);
            }
            break;
        case INPUT_EVENT_QUIT:
            ctx->game->finished = true;
            ctx->game->status = GAME_STATUS_FINISHED;
            break;
        case INPUT_EVENT_TOGGLE_CHAT:
            ctx->chat_mode = !ctx->chat_mode;
            input_set_chat_mode(ctx->chat_mode);
            break;
        default:
            break;
    }
}

static void tui_handle_ai_turn(tui_context_t *ctx) {
    if (!ctx || !ctx->game) {
        return;
    }
    if (ctx->game->mode != GAME_MODE_SINGLE) {
        return;
    }
    if (ctx->game->status != GAME_STATUS_RUNNING || ctx->game->finished) {
        return;
    }
    if (ctx->game->players[game_current_player(ctx->game)] != PLAYER_AI) {
        return;
    }
    move_t move;
    if (game_ai_turn(ctx->game, &move)) {
        char msg[64];
        snprintf(msg, sizeof(msg), "AI moved (%d, %d)", move.row + 1, move.col + 1);
        tui_add_chat(ctx, "System", msg);
        tui_show_message(ctx, "AI played");
    }
}

static void tui_process_network_events(tui_context_t *ctx) {
    if (!ctx || !ctx->network) {
        return;
    }
    pthread_mutex_lock(&ctx->event_mutex);
    for (size_t i = 0; i < ctx->pending_count; ++i) {
        network_event_t *evt = &ctx->pending_events[i];
        switch (evt->type) {
            case NET_EVENT_CHAT:
                tui_add_chat(ctx, ctx->game->nicknames[tui_remote_player_index(ctx->game)],
                             evt->data.chat.message);
                break;
            case NET_EVENT_MOVE:
                if (game_place_stone(ctx->game, evt->data.move.row, evt->data.move.col)) {
                    char msg[64];
                    snprintf(msg, sizeof(msg), "Opponent placed (%d, %d)", evt->data.move.row + 1,
                             evt->data.move.col + 1);
                    tui_add_chat(ctx, "System", msg);
                }
                break;
            case NET_EVENT_UNDO_REQUEST:
                game_request_undo(ctx->game, tui_remote_player_index(ctx->game));
                tui_show_message(ctx, "Undo requested (Y/N)");
                break;
            case NET_EVENT_UNDO_RESPONSE:
                if (evt->data.undo.accepted) {
                    game_apply_undo(ctx->game);
                    tui_add_chat(ctx, "System", "Undo accepted");
                } else {
                    tui_add_chat(ctx, "System", "Undo declined");
                    game_cancel_undo(ctx->game);
                }
                break;
            case NET_EVENT_RESIGN:
                tui_add_chat(ctx, "System", "Opponent resigned");
                game_finish(ctx->game, tui_local_player_index(ctx->game));
                break;
            case NET_EVENT_CONNECTED: {
                int remote_index = tui_remote_player_index(ctx->game);
                snprintf(ctx->game->nicknames[remote_index],
                         CONFIG_MAX_NICKNAME, "%s", evt->data.connected.nickname);
                tui_add_chat(ctx, "System", "Connected");
                break;
            }
            case NET_EVENT_DISCONNECTED:
                tui_add_chat(ctx, "System", "Disconnected");
                break;
            default:
                break;
        }
    }
    ctx->pending_count = 0;
    pthread_mutex_unlock(&ctx->event_mutex);
}

void tui_notify_network_event(tui_context_t *ctx, const network_event_t *event) {
    if (!ctx || !event) {
        return;
    }
    pthread_mutex_lock(&ctx->event_mutex);
    if (ctx->pending_count < sizeof(ctx->pending_events) / sizeof(ctx->pending_events[0])) {
        ctx->pending_events[ctx->pending_count++] = *event;
    }
    pthread_mutex_unlock(&ctx->event_mutex);
}

static void tui_update_info_message(tui_context_t *ctx) {
    if (!ctx) {
        return;
    }
    long now = utils_now_ms();
    pthread_mutex_lock(&ctx->event_mutex);
    if (ctx->info_expires_at > 0 && now > ctx->info_expires_at) {
        ctx->info_message[0] = '\0';
        ctx->info_expires_at = 0;
    }
    pthread_mutex_unlock(&ctx->event_mutex);
}

static void tui_render(tui_context_t *ctx) {
    if (!ctx) {
        return;
    }
    tui_resize_layout(ctx);
    pthread_mutex_lock(&ctx->event_mutex);
    const char *info = ctx->info_message;
    pthread_mutex_unlock(&ctx->event_mutex);
    status_bar_render(ctx->status_win, ctx->game, info, ctx->chat_mode);
    board_view_render(ctx->board_win, ctx->game, ctx->cursor_row, ctx->cursor_col);
    chat_view_render(ctx->chat_win, ctx->chat_log);
    chat_input_render(ctx->input_win, ctx->chat_buffer, ctx->chat_mode);
}

static void tui_check_timeout(tui_context_t *ctx) {
    if (!ctx || !ctx->game) {
        return;
    }
    if (ctx->game->turn_time_limit <= 0) {
        return;
    }
    if (ctx->game->finished || ctx->game->status != GAME_STATUS_RUNNING) {
        return;
    }
    if (game_timer_remaining(ctx->game, game_current_player(ctx->game)) <= 0) {
        int loser = game_current_player(ctx->game);
        if (ctx->network && tui_local_player_index(ctx->game) == loser) {
            network_send_resign(ctx->network);
        }
        game_resign(ctx->game, loser);
        tui_add_chat(ctx, "System", "Time out!");
        tui_show_message(ctx, "Time out!");
    }
}

int tui_run(tui_context_t *ctx, network_session_t *network) {
    if (!ctx) {
        return -1;
    }
    ctx->network = network;
    bool running = true;
    while (running) {
        tui_process_network_events(ctx);
        tui_check_timeout(ctx);
        tui_handle_ai_turn(ctx);
        input_event_t event;
        while (input_poll(&event)) {
            if (event.type == INPUT_EVENT_QUIT) {
                running = false;
            }
            tui_handle_input_event(ctx, &event);
        }
        tui_update_info_message(ctx);
        tui_render(ctx);
        if (ctx->game->finished) {
            tui_show_message(ctx, "Game finished - press Q to quit");
        }
        napms(30);
    }
    return 0;
}
