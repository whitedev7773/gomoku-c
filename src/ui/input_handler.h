#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <ncurses.h>
#include <stdbool.h>

typedef enum {
    INPUT_NONE = 0,
    INPUT_MOVE_UP,
    INPUT_MOVE_DOWN,
    INPUT_MOVE_LEFT,
    INPUT_MOVE_RIGHT,
    INPUT_PLACE_STONE,
    INPUT_CHAT_MODE,
    INPUT_QUIT,
    INPUT_UNDO_REQUEST,
    INPUT_RESIGN
} InputAction;

typedef struct {
    InputAction action;
    int key_code;
    char character;
} InputEvent;

InputEvent input_get_event(WINDOW *win);

bool input_is_arrow_key(int key);

bool input_is_action_key(int key);

InputAction input_map_key_to_action(int key);

#endif // INPUT_HANDLER_H
