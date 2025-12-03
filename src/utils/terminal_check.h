#ifndef TERMINAL_CHECK_H
#define TERMINAL_CHECK_H

#include <stdbool.h>

#define MIN_TERMINAL_WIDTH 100
#define MIN_TERMINAL_HEIGHT 30

typedef struct {
    int width;
    int height;
} TerminalSize;

// Get current terminal size
TerminalSize get_terminal_size(void);

// Check if terminal meets minimum requirements
bool check_terminal_size(void);

// Display terminal size error message
void display_terminal_size_error(TerminalSize current);

#endif // TERMINAL_CHECK_H
