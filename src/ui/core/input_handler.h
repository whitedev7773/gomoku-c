#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <ncurses.h>
#include <stdbool.h>
#include "gamepad_input.h"

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
    bool from_gamepad;  // 게임패드 입력 여부
} InputEvent;

// 게임패드 상태를 포함하는 입력 핸들러
typedef struct {
    GamepadState gamepad;
    bool gamepad_enabled;
} InputHandler;

// 입력 핸들러 초기화
void input_handler_init(InputHandler *handler);

// 입력 핸들러 정리
void input_handler_cleanup(InputHandler *handler);

// 게임패드 포함 입력 이벤트 가져오기
InputEvent input_handler_get_event(InputHandler *handler, WINDOW *win);

// 레거시 함수 (키보드만)
InputEvent input_get_event(WINDOW *win);

bool input_is_arrow_key(int key);

bool input_is_action_key(int key);

InputAction input_map_key_to_action(int key);

#endif // INPUT_HANDLER_H
