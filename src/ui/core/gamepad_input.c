#include "gamepad_input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <errno.h>

#define MAX_JOYSTICK_DEVICES 16
#define STICK_DEADZONE 10000 // 데드존 임계값 (-32768~32767 범위)

// Xbox 컨트롤러인지 확인
static bool is_xbox_controller(const char *device_name)
{
    return strstr(device_name, "Xbox") != NULL ||
           strstr(device_name, "X-Box") != NULL ||
           strstr(device_name, "XBOX") != NULL ||
           strstr(device_name, "Microsoft X-Box") != NULL ||
           strstr(device_name, "Xbox 360") != NULL ||
           strstr(device_name, "Xbox360") != NULL ||
           strstr(device_name, "Xbox One") != NULL ||
           strstr(device_name, "Xbox Series") != NULL;
}

// Xbox 컨트롤러 자동 감지 (js 디바이스)
static int find_xbox_controller_js(void)
{
    char device_path[32];
    char device_name[128];

    // /dev/input/js0 ~ js15까지 순회
    for (int i = 0; i < MAX_JOYSTICK_DEVICES; i++)
    {
        snprintf(device_path, sizeof(device_path), "/dev/input/js%d", i);

        int fd = open(device_path, O_RDONLY | O_NONBLOCK);
        if (fd < 0)
            continue;

        // 디바이스 이름 가져오기
        if (ioctl(fd, JSIOCGNAME(sizeof(device_name)), device_name) < 0)
        {
            close(fd);
            continue;
        }

        // Xbox 컨트롤러 확인
        if (is_xbox_controller(device_name))
        {
            printf("Xbox Controller detected: %s (on %s)\n", device_name, device_path);
            return fd;
        }

        close(fd);
    }

    return -1;
}

// Xbox 컨트롤러 자동 감지 (event 디바이스)
static int find_xbox_controller_event(void)
{
    char device_path[32];
    char device_name[256];

    // /dev/input/event0 ~ event15까지 순회
    for (int i = 0; i < MAX_JOYSTICK_DEVICES; i++)
    {
        snprintf(device_path, sizeof(device_path), "/dev/input/event%d", i);

        int fd = open(device_path, O_RDONLY | O_NONBLOCK);
        if (fd < 0)
            continue;

        // 디바이스 이름 가져오기
        if (ioctl(fd, EVIOCGNAME(sizeof(device_name)), device_name) < 0)
        {
            close(fd);
            continue;
        }

        // Xbox 컨트롤러 확인
        if (is_xbox_controller(device_name))
        {
            printf("Xbox Controller detected: %s (on %s)\n", device_name, device_path);
            return fd;
        }

        close(fd);
    }

    return -1;
}

// Xbox 컨트롤러 자동 감지 (js 우선, event 대체)
static int find_xbox_controller(void)
{
    int fd;

    // 1. js 디바이스 먼저 시도 (조이스틱 전용 API)
    fd = find_xbox_controller_js();
    if (fd >= 0)
        return fd;

    // 2. event 디바이스 시도 (범용 입력 API)
    fd = find_xbox_controller_event();
    if (fd >= 0)
        return fd;

    return -1; // Xbox 컨트롤러를 찾지 못함
}

bool gamepad_init(GamepadState *state)
{
    if (!state)
        return false;

    memset(state, 0, sizeof(GamepadState));
    state->stick_threshold = STICK_DEADZONE;

    // Xbox 컨트롤러 자동 감지
    state->fd = find_xbox_controller();
    if (state->fd < 0)
    {
        // Xbox 컨트롤러가 없으면 연결되지 않은 것으로 처리
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

// [수정] event0 (evdev) 프로토콜을 지원하는 함수
GamepadEvent gamepad_get_event(GamepadState *state)
{
    GamepadEvent event;
    event.type = GAMEPAD_EVENT_NONE;

    if (!state || !state->connected)
        return event;

    // event 인터페이스는 input_event 구조체를 사용합니다 (24 bytes)
    struct input_event ie;
    ssize_t bytes = read(state->fd, &ie, sizeof(ie));

    if (bytes == sizeof(ie))
    {
        // 1. 버튼 입력 (EV_KEY)
        if (ie.type == EV_KEY)
        {
            int btn_index = -1;

            // Linux Input Event 코드를 GamepadState 배열 인덱스로 매핑
            // (Xbox 360 컨트롤러 기준)
            switch (ie.code)
            {
            case BTN_SOUTH:
                btn_index = 0;
                break; // A
            case BTN_EAST:
                btn_index = 1;
                break; // B
            case BTN_WEST:
                btn_index = 2;
                break; // X
            case BTN_NORTH:
                btn_index = 3;
                break; // Y
            case BTN_TL:
                btn_index = 4;
                break; // LB
            case BTN_TR:
                btn_index = 5;
                break; // RB
            case BTN_SELECT:
                btn_index = 6;
                break; // Back
            case BTN_START:
                btn_index = 7;
                break; // Start
            case BTN_MODE:
                btn_index = 8;
                break; // Xbox Button
            case BTN_THUMBL:
                btn_index = 9;
                break; // L3
            case BTN_THUMBR:
                btn_index = 10;
                break; // R3
            // D-Pad가 버튼으로 들어오는 경우도 있음 (설정에 따라 다름)
            default:
                break;
            }

            if (btn_index != -1)
            {
                state->buttons[btn_index] = ie.value; // 0=뗌, 1=누름
                event.type = GAMEPAD_EVENT_BUTTON;
                event.button.button = btn_index;
                event.button.pressed = ie.value;
                return event;
            }
        }
        // 2. 아날로그 축 입력 (EV_ABS)
        else if (ie.type == EV_ABS)
        {
            int axis_index = -1;

            switch (ie.code)
            {
            case ABS_X:
                axis_index = 0;
                break; // L-Stick X
            case ABS_Y:
                axis_index = 1;
                break; // L-Stick Y
            case ABS_RX:
                axis_index = 3;
                break; // R-Stick X (순서 주의)
            case ABS_RY:
                axis_index = 4;
                break; // R-Stick Y
            case ABS_Z:
                axis_index = 2;
                break; // LT (Trigger)
            case ABS_RZ:
                axis_index = 5;
                break; // RT (Trigger)
            case ABS_HAT0X:
                axis_index = 6;
                break; // D-Pad X
            case ABS_HAT0Y:
                axis_index = 7;
                break; // D-Pad Y
            default:
                break;
            }

            if (axis_index != -1)
            {
                state->axes[axis_index] = ie.value;
                event.type = GAMEPAD_EVENT_AXIS;
                event.axis.axis = axis_index;
                event.axis.value = ie.value;
                return event;
            }
        }
        return event; // 처리하지 않는 이벤트 (EV_SYN 등)는 무시
    }

    // 에러 처리
    if (bytes == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
        {
            return event; // 정상적인 대기 상태
        }
        // 진짜 에러
        printf("Error reading event: %s (errno=%d)\n", strerror(errno), errno);
        state->connected = false;
    }
    else if (bytes == 0)
    {
        printf("Controller disconnected (EOF).\n");
        state->connected = false;
    }
    else
    {
        // 바이트 크기가 안 맞을 때 (ex: struct js_event 사이즈로 읽으려 할 때)
        // 여기서 errno 22가 발생했었음. 이제 input_event 크기라 해결될 것임.
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

void gamepad_list_all_devices(void)
{
    char device_path[32];
    char device_name[256];
    int device_count = 0;

    printf("=== All Input Devices ===\n\n");

    // 1. Joystick 디바이스 나열
    printf("Joystick devices (/dev/input/js*):\n");
    bool found_js = false;
    for (int i = 0; i < MAX_JOYSTICK_DEVICES; i++)
    {
        snprintf(device_path, sizeof(device_path), "/dev/input/js%d", i);
        int fd = open(device_path, O_RDONLY | O_NONBLOCK);
        if (fd < 0)
            continue;

        found_js = true;
        device_count++;

        if (ioctl(fd, JSIOCGNAME(sizeof(device_name)), device_name) >= 0)
        {
            bool is_xbox = is_xbox_controller(device_name);
            printf("  [%d] %s\n", i, device_path);
            printf("      Name: %s\n", device_name);
            if (is_xbox)
            {
                printf("      ✓ Xbox Controller detected!\n");
            }
        }
        else
        {
            printf("  [%d] %s (name unavailable)\n", i, device_path);
        }

        close(fd);
    }
    if (!found_js)
    {
        printf("  (no joystick devices found)\n");
    }
    printf("\n");

    // 2. Event 디바이스 나열
    printf("Event devices (/dev/input/event*):\n");
    bool found_event = false;
    for (int i = 0; i < MAX_JOYSTICK_DEVICES; i++)
    {
        snprintf(device_path, sizeof(device_path), "/dev/input/event%d", i);
        int fd = open(device_path, O_RDONLY | O_NONBLOCK);
        if (fd < 0)
            continue;

        found_event = true;
        device_count++;

        if (ioctl(fd, EVIOCGNAME(sizeof(device_name)), device_name) >= 0)
        {
            bool is_xbox = is_xbox_controller(device_name);
            printf("  [%d] %s\n", i, device_path);
            printf("      Name: %s\n", device_name);
            if (is_xbox)
            {
                printf("      ✓ Xbox Controller detected!\n");
            }
        }
        else
        {
            printf("  [%d] %s (name unavailable)\n", i, device_path);
        }

        close(fd);
    }
    if (!found_event)
    {
        printf("  (no event devices found)\n");
    }
    printf("\n");

    printf("Total devices found: %d\n", device_count);
    printf("=========================\n\n");
}
