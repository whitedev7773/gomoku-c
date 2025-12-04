#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "../ui/core/gamepad_input.h"

static volatile bool running = true;

void signal_handler(int sig)
{
    (void)sig;
    running = false;
}

void print_button_state(const GamepadState *state)
{
    printf("Buttons: ");

    if (gamepad_is_button_pressed(state, GAMEPAD_BTN_A))
        printf("[A] ");
    if (gamepad_is_button_pressed(state, GAMEPAD_BTN_B))
        printf("[B] ");
    if (gamepad_is_button_pressed(state, GAMEPAD_BTN_X))
        printf("[X] ");
    if (gamepad_is_button_pressed(state, GAMEPAD_BTN_Y))
        printf("[Y] ");
    if (gamepad_is_button_pressed(state, GAMEPAD_BTN_LB))
        printf("[LB] ");
    if (gamepad_is_button_pressed(state, GAMEPAD_BTN_RB))
        printf("[RB] ");
    if (gamepad_is_button_pressed(state, GAMEPAD_BTN_BACK))
        printf("[BACK] ");
    if (gamepad_is_button_pressed(state, GAMEPAD_BTN_START))
        printf("[START] ");
    if (gamepad_is_button_pressed(state, GAMEPAD_BTN_LSTICK))
        printf("[L3] ");
    if (gamepad_is_button_pressed(state, GAMEPAD_BTN_RSTICK))
        printf("[R3] ");

    printf("\n");
}

void print_axis_state(const GamepadState *state)
{
    int lx = gamepad_get_axis(state, GAMEPAD_AXIS_LX);
    int ly = gamepad_get_axis(state, GAMEPAD_AXIS_LY);
    int rx = gamepad_get_axis(state, GAMEPAD_AXIS_RX);
    int ry = gamepad_get_axis(state, GAMEPAD_AXIS_RY);
    int lt = gamepad_get_axis(state, GAMEPAD_AXIS_LT);
    int rt = gamepad_get_axis(state, GAMEPAD_AXIS_RT);
    int dpad_x = gamepad_get_axis(state, GAMEPAD_AXIS_DPAD_X);
    int dpad_y = gamepad_get_axis(state, GAMEPAD_AXIS_DPAD_Y);

    printf("Left Stick: (%6d, %6d)  ", lx, ly);
    printf("Right Stick: (%6d, %6d)\n", rx, ry);
    printf("Triggers: LT=%6d RT=%6d  ", lt, rt);
    printf("D-Pad: (%2d, %2d)\n", dpad_x, dpad_y);
}

int main(void)
{
    printf("=== Xbox Controller Detection Test ===\n\n");

    // SIGINT 핸들러 설정 (Ctrl+C)
    signal(SIGINT, signal_handler);

    // 모든 입력 장치 나열
    gamepad_list_all_devices();

    GamepadState gamepad;

    printf("Searching for Xbox Controller...\n");
    if (!gamepad_init(&gamepad))
    {
        fprintf(stderr, "Failed to initialize gamepad subsystem\n");
        return 1;
    }

    if (!gamepad_is_connected(&gamepad))
    {
        printf("\n❌ Xbox Controller NOT detected\n");
        printf("   Please connect an Xbox Controller and try again.\n\n");
        printf("   Supported devices:\n");
        printf("   - Xbox 360 Controller\n");
        printf("   - Xbox One Controller\n");
        printf("   - Xbox Series X|S Controller\n");
        printf("   - Any Microsoft Xbox compatible controller\n\n");
        gamepad_cleanup(&gamepad);
        return 0;
    }

    printf("\n✓ Xbox Controller detected successfully!\n\n");
    printf("Press Ctrl+C to exit\n");
    printf("Try pressing buttons and moving sticks...\n");
    printf("----------------------------------------\n\n");

    bool any_input = false;

    while (running)
    {
        GamepadEvent event = gamepad_get_event(&gamepad);

        if (!gamepad_is_connected(&gamepad))
        {
            printf("\n⚠ Xbox Controller disconnected!\n");
            break;
        }

        if (event.type == GAMEPAD_EVENT_BUTTON)
        {
            any_input = true;
            printf("\r"); // 커서를 줄 처음으로 이동
            printf("Button Event: #%d %s                    \n",
                   event.button.button,
                   event.button.pressed ? "PRESSED" : "RELEASED");
            print_button_state(&gamepad);
        }
        else if (event.type == GAMEPAD_EVENT_AXIS)
        {
            any_input = true;
            printf("\r"); // 커서를 줄 처음으로 이동
            printf("Axis Event: #%d = %6d                    \n",
                   event.axis.axis,
                   event.axis.value);
            print_axis_state(&gamepad);
        }

        if (any_input)
        {
            printf("----------------------------------------\n");
            any_input = false;
        }

        usleep(10000); // 10ms 대기
    }

    printf("\n\nCleaning up...\n");
    gamepad_cleanup(&gamepad);
    printf("Test completed.\n");

    return 0;
}
