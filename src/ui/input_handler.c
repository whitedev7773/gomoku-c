#include "input_handler.h"

InputAction input_map_key_to_action(int key) {
    switch (key) {
        case KEY_UP:
            return INPUT_MOVE_UP;
        case KEY_DOWN:
            return INPUT_MOVE_DOWN;
        case KEY_LEFT:
            return INPUT_MOVE_LEFT;
        case KEY_RIGHT:
            return INPUT_MOVE_RIGHT;
        case ' ':
        case '\n':
        case KEY_ENTER:
            return INPUT_PLACE_STONE;
        case 't':
        case 'T':
            return INPUT_CHAT_MODE;
        case 'q':
        case 'Q':
            return INPUT_QUIT;
        case 'u':
        case 'U':
            return INPUT_UNDO_REQUEST;
        case 'r':
        case 'R':
            return INPUT_RESIGN;
        default:
            return INPUT_NONE;
    }
}

bool input_is_arrow_key(int key) {
    return (key == KEY_UP || key == KEY_DOWN ||
            key == KEY_LEFT || key == KEY_RIGHT);
}

bool input_is_action_key(int key) {
    return (key == ' ' || key == '\n' || key == KEY_ENTER);
}

InputEvent input_get_event(WINDOW *win) {
    InputEvent event = {INPUT_NONE, 0, '\0'};

    if (!win) return event;

    int key = wgetch(win);
    event.key_code = key;

    if (key >= 0 && key < 256) {
        event.character = (char)key;
    }

    event.action = input_map_key_to_action(key);

    return event;
}
