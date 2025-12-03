#include "gamepad_input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <errno.h>

#define JOYSTICK_DEVICE "/dev/input/js0"
#define STICK_DEADZONE 10000  // 데드존 임계값 (-32768~32767 범위)

bool gamepad_init(GamepadState *state)
{
    if (!state)
        return false;

    memset(state, 0, sizeof(GamepadState));
    state->stick_threshold = STICK_DEADZONE;

    // 조이스틱 장치 열기
    state->fd = open(JOYSTICK_DEVICE, O_RDONLY | O_NONBLOCK);
    if (state->fd < 0)
    {
        // 장치가 없으면 연결되지 않은 것으로 처리 (에러 아님)
        state->connected = false;
        return true; // 게임패드 없이도 게임은 진행 가능
    }

    state->connected = true;
    return true;
}

void gamepad_cleanup(GamepadState *state)
{
    if (!state)
        return;

    if (state->fd >= 0)
    {
        close(state->fd);
        state->fd = -1;
    }

    state->connected = false;
}

bool gamepad_is_connected(const GamepadState *state)
{
    return state && state->connected;
}

GamepadEvent gamepad_get_event(GamepadState *state)
{
    GamepadEvent event;
    event.type = GAMEPAD_EVENT_NONE;

    if (!state || !state->connected)
        return event;

    struct js_event js;
    ssize_t bytes = read(state->fd, &js, sizeof(js));

    if (bytes != sizeof(js))
    {
        // 에러 또는 데이터 없음
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            // 실제 에러 발생 (연결 끊김 등)
            state->connected = false;
        }
        return event;
    }

    // 초기화 이벤트 무시
    if (js.type & JS_EVENT_INIT)
        return event;

    // 버튼 이벤트
    if (js.type == JS_EVENT_BUTTON)
    {
        if (js.number < 16)
        {
            state->buttons[js.number] = js.value;
            event.type = GAMEPAD_EVENT_BUTTON;
            event.button.button = js.number;
            event.button.pressed = js.value;
        }
    }
    // 축 이벤트
    else if (js.type == JS_EVENT_AXIS)
    {
        if (js.number < 8)
        {
            state->axes[js.number] = js.value;
            event.type = GAMEPAD_EVENT_AXIS;
            event.axis.axis = js.number;
            event.axis.value = js.value;
        }
    }

    return event;
}

bool gamepad_is_button_pressed(const GamepadState *state, GamepadButton button)
{
    if (!state || !state->connected)
        return false;

    if (button < 0 || button >= 16)
        return false;

    return state->buttons[button];
}

int gamepad_get_axis(const GamepadState *state, GamepadAxis axis)
{
    if (!state || !state->connected)
        return 0;

    if (axis < 0 || axis >= 8)
        return 0;

    return state->axes[axis];
}
