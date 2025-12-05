#ifndef TERMINAL_CHECK_H
#define TERMINAL_CHECK_H

#include <stdbool.h>

#define MIN_TERMINAL_WIDTH 100
#define MIN_TERMINAL_HEIGHT 30

typedef struct
{
    int width;
    int height;
} TerminalSize;

// Get current terminal size
TerminalSize get_terminal_size(void);

// Check if terminal meets minimum requirements
bool check_terminal_size(void);

// Display terminal size error message
void display_terminal_size_error(TerminalSize current);

// 터미널 크기 변경 감지 결과
typedef enum
{
    TERMINAL_SIZE_OK = 0,    // 크기 충분
    TERMINAL_SIZE_TOO_SMALL, // 크기 부족
    TERMINAL_SIZE_RESTORED   // 크기 복원됨
} TerminalSizeStatus;

// 게임 중 터미널 크기 체크 (ncurses 환경에서 사용)
TerminalSizeStatus check_terminal_size_ingame(void);

#endif // TERMINAL_CHECK_H
