#ifndef GAMEPAD_INPUT_H
#define GAMEPAD_INPUT_H

#include <stdbool.h>

// Xbox Controller 버튼 매핑
typedef enum {
    GAMEPAD_BTN_A = 0,          // 돌 놓기 (Space)
    GAMEPAD_BTN_B = 1,          // 취소/뒤로 (ESC)
    GAMEPAD_BTN_X = 2,          // 무르기 요청
    GAMEPAD_BTN_Y = 3,          // 기권
    GAMEPAD_BTN_LB = 4,         // 왼쪽 범퍼
    GAMEPAD_BTN_RB = 5,         // 오른쪽 범퍼
    GAMEPAD_BTN_BACK = 6,       // Back/Select (나가기)
    GAMEPAD_BTN_START = 7,      // Start (채팅 모드)
    GAMEPAD_BTN_LSTICK = 8,     // 왼쪽 스틱 클릭
    GAMEPAD_BTN_RSTICK = 9,     // 오른쪽 스틱 클릭
} GamepadButton;

// 게임패드 축 매핑
typedef enum {
    GAMEPAD_AXIS_LX = 0,        // 왼쪽 스틱 X축
    GAMEPAD_AXIS_LY = 1,        // 왼쪽 스틱 Y축
    GAMEPAD_AXIS_LT = 2,        // 왼쪽 트리거
    GAMEPAD_AXIS_RX = 3,        // 오른쪽 스틱 X축
    GAMEPAD_AXIS_RY = 4,        // 오른쪽 스틱 Y축
    GAMEPAD_AXIS_RT = 5,        // 오른쪽 트리거
    GAMEPAD_AXIS_DPAD_X = 6,    // D-Pad X축
    GAMEPAD_AXIS_DPAD_Y = 7,    // D-Pad Y축
} GamepadAxis;

// 게임패드 이벤트 타입
typedef enum {
    GAMEPAD_EVENT_BUTTON,
    GAMEPAD_EVENT_AXIS,
    GAMEPAD_EVENT_NONE
} GamepadEventType;

// 게임패드 이벤트
typedef struct {
    GamepadEventType type;
    union {
        struct {
            int button;
            bool pressed;
        } button;
        struct {
            int axis;
            int value;
        } axis;
    };
} GamepadEvent;

// 게임패드 상태
typedef struct {
    int fd;                     // 파일 디스크립터
    bool connected;             // 연결 상태
    bool buttons[16];           // 버튼 상태
    int axes[8];                // 축 상태

    // 스틱 입력 디바운스용
    bool stick_moved;           // 스틱이 이동했는가
    int last_stick_x;
    int last_stick_y;
    int stick_threshold;        // 데드존 임계값
} GamepadState;

// 게임패드 초기화
bool gamepad_init(GamepadState *state);

// 게임패드 정리
void gamepad_cleanup(GamepadState *state);

// 게임패드 연결 확인
bool gamepad_is_connected(const GamepadState *state);

// 게임패드 이벤트 읽기 (논블로킹)
GamepadEvent gamepad_get_event(GamepadState *state);

// 게임패드 버튼 상태 확인
bool gamepad_is_button_pressed(const GamepadState *state, GamepadButton button);

// 게임패드 축 값 가져오기
int gamepad_get_axis(const GamepadState *state, GamepadAxis axis);

#endif // GAMEPAD_INPUT_H
