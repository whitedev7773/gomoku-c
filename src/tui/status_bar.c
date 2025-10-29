#include <ncurses.h>
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "utils.h"

void status_bar_render(WINDOW *win, const game_t *game, const char *info_message, bool chat_mode) {
    if (!win || !game) {
        return;
    }
    werase(win);
    int width = getmaxx(win);

    const char *mode_text = "";
    switch (game->mode) {
        case GAME_MODE_SINGLE:
            mode_text = "Single (AI)";
            break;
        case GAME_MODE_MULTI_HOST:
            mode_text = "LAN - Host";
            break;
        case GAME_MODE_MULTI_CLIENT:
            mode_text = "LAN - Client";
            break;
        default:
            mode_text = "Unknown";
            break;
    }

    char timer_buf_p0[8] = "";
    char timer_buf_p1[8] = "";
    if (game->turn_time_limit > 0) {
        utils_format_time(game_timer_remaining(game, 0), timer_buf_p0, sizeof(timer_buf_p0));
        utils_format_time(game_timer_remaining(game, 1), timer_buf_p1, sizeof(timer_buf_p1));
    } else {
        snprintf(timer_buf_p0, sizeof(timer_buf_p0), "--:--");
        snprintf(timer_buf_p1, sizeof(timer_buf_p1), "--:--");
    }

    mvwprintw(win, 0, 1, "%s | %s (X) [%s] vs %s (O) [%s]", mode_text,
              game->nicknames[0], timer_buf_p0,
              game->nicknames[1], timer_buf_p1);

    const char *turn_name = game->nicknames[game->current_player];
    const char *stone_name = game_current_stone(game) == STONE_BLACK ? "X" : "O";
    mvwprintw(win, 1, 1, "Turn: %s (%s)%s", turn_name, stone_name,
              chat_mode ? "  [Chat]" : "");

    if (info_message && info_message[0] != '\0') {
        int msg_len = (int)strlen(info_message);
        int pos = width - msg_len - 2;
        if (pos < 1) {
            pos = 1;
        }
        mvwprintw(win, 0, pos, "%s", info_message);
    }

    mvwprintw(win, 2, 1, "[U]Undo  [Y/N]Accept undo  [R]Resign  [Q]Quit");
    wrefresh(win);
}
