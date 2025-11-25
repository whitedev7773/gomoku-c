#include "terminal_check.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

TerminalSize get_terminal_size(void) {
    TerminalSize size = {0, 0};
    struct winsize w;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        size.width = w.ws_col;
        size.height = w.ws_row;
    }

    return size;
}

bool check_terminal_size(void) {
    TerminalSize size = get_terminal_size();
    return (size.width >= MIN_TERMINAL_WIDTH && size.height >= MIN_TERMINAL_HEIGHT);
}

void display_terminal_size_error(TerminalSize current) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║           TERMINAL SIZE REQUIREMENT ERROR                 ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║                                                            ║\n");
    printf("║  Current Size:  %3d x %3d                                ║\n", current.width, current.height);
    printf("║  Required Size: %3d x %3d                                ║\n", MIN_TERMINAL_WIDTH, MIN_TERMINAL_HEIGHT);
    printf("║                                                            ║\n");
    printf("║  Please resize your terminal window to meet the           ║\n");
    printf("║  minimum requirements and try again.                      ║\n");
    printf("║                                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}
