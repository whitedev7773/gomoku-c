#include "input_handler.h"
#include <string.h>

#define STICK_MOVE_THRESHOLD 15000  // 스틱 이동 감지 임계값

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
    InputEvent event = {INPUT_NONE, 0, '\0', false};

    if (!win) return event;

    int key = wgetch(win);
    event.key_code = key;

    if (key >= 0 && key < 256) {
        event.character = (char)key;
    }

    event.action = input_map_key_to_action(key);

    return event;
}

// 입력 핸들러 초기화
void input_handler_init(InputHandler *handler) {
    if (!handler) return;

    memset(handler, 0, sizeof(InputHandler));
    handler->gamepad_enabled = gamepad_init(&handler->gamepad);
}

// 입력 핸들러 정리
void input_handler_cleanup(InputHandler *handler) {
    if (!handler) return;

    if (handler->gamepad_enabled) {
        gamepad_cleanup(&handler->gamepad);
    }
}

// 게임패드 버튼을 InputAction으로 매핑
static InputAction gamepad_button_to_action(GamepadButton button) {
    switch (button) {
        case GAMEPAD_BTN_A:
            return INPUT_PLACE_STONE;
        case GAMEPAD_BTN_B:
            return INPUT_QUIT;
        case GAMEPAD_BTN_X:
            return INPUT_UNDO_REQUEST;
        case GAMEPAD_BTN_Y:
            return INPUT_RESIGN;
        case GAMEPAD_BTN_START:
            return INPUT_CHAT_MODE;
        case GAMEPAD_BTN_BACK:
            return INPUT_QUIT;
        default:
            return INPUT_NONE;
    }
}

// 게임패드 입력 처리
static InputEvent process_gamepad_input(InputHandler *handler) {
    InputEvent event = {INPUT_NONE, 0, '\0', true};

    if (!handler || !handler->gamepad_enabled) {
        return event;
    }

    GamepadEvent gp_event = gamepad_get_event(&handler->gamepad);

    // 버튼 이벤트 처리
    if (gp_event.type == GAMEPAD_EVENT_BUTTON && gp_event.button.pressed) {
        event.action = gamepad_button_to_action(gp_event.button.button);
        return event;
    }

    // 스틱/D-Pad 이벤트 처리 (연속 입력 방지)
    if (gp_event.type == GAMEPAD_EVENT_AXIS) {
        // 왼쪽 스틱 X축
        if (gp_event.axis.axis == GAMEPAD_AXIS_LX) {
            if (gp_event.axis.value > STICK_MOVE_THRESHOLD && !handler->gamepad.stick_moved) {
                handler->gamepad.stick_moved = true;
                event.action = INPUT_MOVE_RIGHT;
                return event;
            } else if (gp_event.axis.value < -STICK_MOVE_THRESHOLD && !handler->gamepad.stick_moved) {
                handler->gamepad.stick_moved = true;
                event.action = INPUT_MOVE_LEFT;
                return event;
            } else if (gp_event.axis.value > -STICK_MOVE_THRESHOLD && gp_event.axis.value < STICK_MOVE_THRESHOLD) {
                handler->gamepad.stick_moved = false;
            }
        }
        // 왼쪽 스틱 Y축
        else if (gp_event.axis.axis == GAMEPAD_AXIS_LY) {
            if (gp_event.axis.value > STICK_MOVE_THRESHOLD && !handler->gamepad.stick_moved) {
                handler->gamepad.stick_moved = true;
                event.action = INPUT_MOVE_DOWN;
                return event;
            } else if (gp_event.axis.value < -STICK_MOVE_THRESHOLD && !handler->gamepad.stick_moved) {
                handler->gamepad.stick_moved = true;
                event.action = INPUT_MOVE_UP;
                return event;
            } else if (gp_event.axis.value > -STICK_MOVE_THRESHOLD && gp_event.axis.value < STICK_MOVE_THRESHOLD) {
                handler->gamepad.stick_moved = false;
            }
        }
        // D-Pad X축
        else if (gp_event.axis.axis == GAMEPAD_AXIS_DPAD_X) {
            if (gp_event.axis.value > 0) {
                event.action = INPUT_MOVE_RIGHT;
                return event;
            } else if (gp_event.axis.value < 0) {
                event.action = INPUT_MOVE_LEFT;
                return event;
            }
        }
        // D-Pad Y축
        else if (gp_event.axis.axis == GAMEPAD_AXIS_DPAD_Y) {
            if (gp_event.axis.value > 0) {
                event.action = INPUT_MOVE_DOWN;
                return event;
            } else if (gp_event.axis.value < 0) {
                event.action = INPUT_MOVE_UP;
                return event;
            }
        }
    }

    return event;
}

// 게임패드 포함 입력 이벤트 가져오기
InputEvent input_handler_get_event(InputHandler *handler, WINDOW *win) {
    InputEvent event = {INPUT_NONE, 0, '\0', false};

    if (!handler || !win) {
        return event;
    }

    // 게임패드 입력 먼저 확인
    if (handler->gamepad_enabled) {
        event = process_gamepad_input(handler);
        if (event.action != INPUT_NONE) {
            return event;
        }
    }

    // 키보드 입력 처리
    int key = wgetch(win);
    event.key_code = key;

    if (key >= 0 && key < 256) {
        event.character = (char)key;
    }

    event.action = input_map_key_to_action(key);

    return event;
}
