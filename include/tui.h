#ifndef TUI_H
#define TUI_H

#include "chat.h"
#include "game.h"
#include "network.h"

typedef struct tui_context tui_context_t;

tui_context_t *tui_create(game_t *game, chat_log_t *chat_log);
void tui_destroy(tui_context_t *ctx);
int tui_run(tui_context_t *ctx, network_session_t *network);
void tui_notify_network_event(tui_context_t *ctx, const network_event_t *event);

#endif // TUI_H
