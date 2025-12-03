#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <ncurses.h>
#include <stdbool.h>

// Window dimensions based on 100x30 layout
#define UI_MIN_WIDTH 100
#define UI_MIN_HEIGHT 30

// Layout constants
#define BOARD_WINDOW_WIDTH 51
#define BOARD_WINDOW_HEIGHT 26
#define BOARD_WINDOW_X 0
#define BOARD_WINDOW_Y 0

#define INFO_WINDOW_WIDTH 49
#define INFO_WINDOW_HEIGHT 2
#define INFO_WINDOW_X 51
#define INFO_WINDOW_Y 0

#define CHAT_WINDOW_WIDTH 49
#define CHAT_WINDOW_HEIGHT 21
#define CHAT_WINDOW_X 51
#define CHAT_WINDOW_Y 2

#define CHAT_INPUT_WIDTH 49
#define CHAT_INPUT_HEIGHT 3
#define CHAT_INPUT_X 51
#define CHAT_INPUT_Y 23

#define BOTTOM_WINDOW_WIDTH 100
#define BOTTOM_WINDOW_HEIGHT 4
#define BOTTOM_WINDOW_X 0
#define BOTTOM_WINDOW_Y 26

typedef struct {
    WINDOW *board_win;
    WINDOW *info_win;
    WINDOW *chat_win;
    WINDOW *chat_input_win;
    WINDOW *bottom_win;
    bool initialized;
} UIManager;

bool ui_manager_init(UIManager *manager);

void ui_manager_cleanup(UIManager *manager);

void ui_manager_refresh_all(UIManager *manager);

void ui_manager_clear_all(UIManager *manager);

bool ui_manager_check_terminal_size(void);

#endif // UI_MANAGER_H
