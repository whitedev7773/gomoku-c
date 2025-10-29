#include <ncurses.h>
#include <string.h>

#include "chat.h"

void chat_view_render(WINDOW *win, const chat_log_t *log) {
    if (!win) {
        return;
    }
    werase(win);
    box(win, 0, 0);
    if (!log) {
        wrefresh(win);
        return;
    }
    int height = getmaxy(win) - 2;
    int row = height;
    size_t index = 0;
    while (row >= 0) {
        const chat_message_t *msg = chat_log_latest(log, index);
        if (!msg) {
            break;
        }
        mvwprintw(win, row + 1, 1, "%s: %s", msg->sender, msg->text);
        row--;
        index++;
    }
    wrefresh(win);
}

void chat_input_render(WINDOW *win, const char *buffer, bool active) {
    if (!win) {
        return;
    }
    werase(win);
    box(win, 0, 0);
    mvwprintw(win, 1, 1, "> %s", buffer ? buffer : "");
    if (active) {
        int x = (int)(strlen(buffer ? buffer : "") + 3);
        wmove(win, 1, x);
        curs_set(1);
    } else {
        curs_set(0);
    }
    wrefresh(win);
}
