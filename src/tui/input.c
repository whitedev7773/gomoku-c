#include "input.h"

#include <ctype.h>
#include <ncurses.h>

static bool chat_mode_enabled = false;

void input_init(void) {
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    noecho();
    curs_set(0);
}

void input_shutdown(void) {
    nodelay(stdscr, FALSE);
}

void input_set_chat_mode(bool enabled) {
    chat_mode_enabled = enabled;
}

static bool handle_chat_key(int ch, input_event_t *event) {
    if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
        event->type = INPUT_EVENT_CHAT_BACKSPACE;
        return true;
    }
    if (ch == '\n' || ch == '\r') {
        event->type = INPUT_EVENT_CHAT_SEND;
        return true;
    }
    if (isprint(ch)) {
        event->type = INPUT_EVENT_CHAT_APPEND;
        event->data.chat.ch = (char)ch;
        return true;
    }
    return false;
}

bool input_poll(input_event_t *event) {
    if (!event) {
        return false;
    }
    int ch = getch();
    if (ch == ERR) {
        return false;
    }

    if (chat_mode_enabled) {
        if (handle_chat_key(ch, event)) {
            return true;
        }
    }

    switch (ch) {
        case KEY_UP:
            event->type = INPUT_EVENT_MOVE_CURSOR;
            event->data.move.drow = -1;
            event->data.move.dcol = 0;
            return true;
        case KEY_DOWN:
            event->type = INPUT_EVENT_MOVE_CURSOR;
            event->data.move.drow = 1;
            event->data.move.dcol = 0;
            return true;
        case KEY_LEFT:
            event->type = INPUT_EVENT_MOVE_CURSOR;
            event->data.move.drow = 0;
            event->data.move.dcol = -1;
            return true;
        case KEY_RIGHT:
            event->type = INPUT_EVENT_MOVE_CURSOR;
            event->data.move.drow = 0;
            event->data.move.dcol = 1;
            return true;
        case '\n':
        case '\r':
        case ' ':
            event->type = chat_mode_enabled ? INPUT_EVENT_CHAT_SEND : INPUT_EVENT_PLACE_STONE;
            return true;
        case '\t':
            event->type = INPUT_EVENT_TOGGLE_CHAT;
            return true;
        case 'u':
        case 'U':
            event->type = INPUT_EVENT_UNDO_REQUEST;
            return true;
        case 'y':
        case 'Y':
            event->type = INPUT_EVENT_UNDO_ACCEPT;
            return true;
        case 'n':
        case 'N':
            event->type = INPUT_EVENT_UNDO_DECLINE;
            return true;
        case 'r':
        case 'R':
            event->type = INPUT_EVENT_RESIGN;
            return true;
        case 'q':
        case 'Q':
            event->type = INPUT_EVENT_QUIT;
            return true;
        default:
            if (chat_mode_enabled && isprint(ch)) {
                event->type = INPUT_EVENT_CHAT_APPEND;
                event->data.chat.ch = (char)ch;
                return true;
            }
            break;
    }
    return false;
}
