#include "input_handler.h"
#include <string.h>

// D-Pad만 사용 (스틱 비활성화)
// 모든 버튼은 버튼을 뗄 때까지 한 번만 입력

InputAction input_map_key_to_action(int key)
{
    switch (key)
    {
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

bool input_is_arrow_key(int key)
{
    return (key == KEY_UP || key == KEY_DOWN ||
            key == KEY_LEFT || key == KEY_RIGHT);
}

bool input_is_action_key(int key)
{
    return (key == ' ' || key == '\n' || key == KEY_ENTER);
}

InputEvent input_get_event(WINDOW *win)
{
    InputEvent event = {INPUT_NONE, 0, '\0', false};

    if (!win)
        return event;

    int key = wgetch(win);
    event.key_code = key;

    if (key >= 0 && key < 256)
    {
        event.character = (char)key;
    }

    event.action = input_map_key_to_action(key);

    return event;
}

// 입력 핸들러 초기화
void input_handler_init(InputHandler *handler)
{
    if (!handler)
        return;

    memset(handler, 0, sizeof(InputHandler));
    handler->gamepad_enabled = gamepad_init(&handler->gamepad);
}

// 입력 핸들러 정리
void input_handler_cleanup(InputHandler *handler)
{
    if (!handler)
        return;

    if (handler->gamepad_enabled)
    {
        gamepad_cleanup(&handler->gamepad);
    }
}

// 게임패드 버튼을 InputAction으로 매핑 (XYAB만 사용)
static InputAction gamepad_button_to_action(GamepadButton button)
{
    switch (button)
    {
    case GAMEPAD_BTN_A:
        return INPUT_PLACE_STONE;
    case GAMEPAD_BTN_B:
        return INPUT_QUIT;
    case GAMEPAD_BTN_X:
        return INPUT_UNDO_REQUEST;
    case GAMEPAD_BTN_Y:
        return INPUT_RESIGN;
    default:
        return INPUT_NONE;
    }
}

// D-Pad 입력 처리 (버튼을 뗄 때까지 한 번만 입력)
static InputAction check_dpad_input(InputHandler *handler, int dpad_x, int dpad_y)
{
    GamepadState *gp = &handler->gamepad;

    // D-Pad 상태 변화 감지 (눌렀을 때만 반응)
    bool dpad_changed = (dpad_x != gp->dpad_x || dpad_y != gp->dpad_y);

    if (dpad_changed)
    {
        int prev_x = gp->dpad_x;
        int prev_y = gp->dpad_y;
        gp->dpad_x = dpad_x;
        gp->dpad_y = dpad_y;

        // 새로 눌린 방향만 입력 (0에서 다른 값으로 변할 때)
        if (dpad_x != 0 && prev_x == 0)
        {
            return (dpad_x > 0) ? INPUT_MOVE_RIGHT : INPUT_MOVE_LEFT;
        }
        if (dpad_y != 0 && prev_y == 0)
        {
            return (dpad_y > 0) ? INPUT_MOVE_DOWN : INPUT_MOVE_UP;
        }
    }

    return INPUT_NONE;
}

// 게임패드 입력 처리 (D-Pad + XYAB 버튼만)
static InputEvent process_gamepad_input(InputHandler *handler)
{
    InputEvent event = {INPUT_NONE, 0, '\0', true};

    if (!handler || !handler->gamepad_enabled)
    {
        return event;
    }

    GamepadState *gp = &handler->gamepad;

    // 모든 대기 중인 이벤트 처리 (폴링)
    GamepadEvent gp_event;
    int events_processed = 0;
    const int MAX_EVENTS_PER_FRAME = 16;

    while (events_processed < MAX_EVENTS_PER_FRAME)
    {
        gp_event = gamepad_get_event(gp);

        if (gp_event.type == GAMEPAD_EVENT_NONE)
        {
            break;
        }

        events_processed++;

        // 버튼 이벤트 처리 (누를 때만, XYAB만)
        if (gp_event.type == GAMEPAD_EVENT_BUTTON && gp_event.button.pressed)
        {
            event.action = gamepad_button_to_action(gp_event.button.button);
            if (event.action != INPUT_NONE)
            {
                return event;
            }
        }
    }

    // D-Pad 입력 확인 (한 번만 입력)
    int dpad_x = gamepad_get_axis(gp, GAMEPAD_AXIS_DPAD_X);
    int dpad_y = gamepad_get_axis(gp, GAMEPAD_AXIS_DPAD_Y);
    event.action = check_dpad_input(handler, dpad_x, dpad_y);

    return event;
}

// 게임패드 포함 입력 이벤트 가져오기
InputEvent input_handler_get_event(InputHandler *handler, WINDOW *win)
{
    InputEvent event = {INPUT_NONE, 0, '\0', false};

    if (!handler || !win)
    {
        return event;
    }

    // 게임패드 입력 먼저 확인
    if (handler->gamepad_enabled)
    {
        event = process_gamepad_input(handler);
        if (event.action != INPUT_NONE)
        {
            return event;
        }
    }

    // 키보드 입력 처리
    int key = wgetch(win);
    event.key_code = key;

    if (key >= 0 && key < 256)
    {
        event.character = (char)key;
    }

    event.action = input_map_key_to_action(key);

    return event;
}
