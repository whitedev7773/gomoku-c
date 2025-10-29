#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

typedef enum {
    INPUT_EVENT_NONE = 0,
    INPUT_EVENT_MOVE_CURSOR,
    INPUT_EVENT_PLACE_STONE,
    INPUT_EVENT_CHAT_APPEND,
    INPUT_EVENT_CHAT_BACKSPACE,
    INPUT_EVENT_CHAT_SEND,
    INPUT_EVENT_UNDO_REQUEST,
    INPUT_EVENT_UNDO_ACCEPT,
    INPUT_EVENT_UNDO_DECLINE,
    INPUT_EVENT_RESIGN,
    INPUT_EVENT_QUIT,
    INPUT_EVENT_TOGGLE_CHAT
} input_event_type_t;

typedef struct {
    input_event_type_t type;
    union {
        struct {
            int drow;
            int dcol;
        } move;
        struct {
            char ch;
        } chat;
    } data;
} input_event_t;

void input_init(void);
bool input_poll(input_event_t *event);
void input_shutdown(void);
void input_set_chat_mode(bool enabled);

#endif // INPUT_H
