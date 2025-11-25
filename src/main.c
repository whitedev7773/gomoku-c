#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils/terminal_check.h"
#include "utils/arg_parser.h"

int main(int argc, char *argv[]) {
    // Parse command line arguments
    ParsedArgs args = parse_arguments(argc, argv);

    if (!args.valid) {
        if (args.error_message[0] != '\0') {
            printf("%s\n", args.error_message);
            display_usage(argv[0]);
        }
        return 1;
    }

    // Check terminal size
    TerminalSize current_size = get_terminal_size();

    while (!check_terminal_size()) {
        system("clear");
        display_terminal_size_error(current_size);
        printf("Press Ctrl+C to exit or resize terminal and press Enter to continue...\n");
        getchar();
        current_size = get_terminal_size();
    }

    system("clear");

    // Handle different game modes
    switch (args.mode) {
        case MODE_MENU:
            printf("=== GOMOKU GAME ===\n");
            printf("Starting menu mode...\n");
            printf("(Menu implementation coming in Phase 3)\n");
            break;

        case MODE_SINGLEPLAY_EASY:
            printf("=== GOMOKU GAME ===\n");
            printf("Starting singleplay - Easy mode...\n");
            printf("(Singleplay implementation coming in Phase 4)\n");
            break;

        case MODE_SINGLEPLAY_HARD:
            printf("=== GOMOKU GAME ===\n");
            printf("Starting singleplay - Hard mode...\n");
            printf("(Singleplay implementation coming in Phase 4)\n");
            break;

        case MODE_MULTIPLAY_HOST:
            printf("=== GOMOKU GAME ===\n");
            printf("Starting multiplayer as host...\n");
            printf("(Multiplayer implementation coming in Phase 5)\n");
            break;

        case MODE_MULTIPLAY_CLIENT:
            printf("=== GOMOKU GAME ===\n");
            printf("Connecting to %s:%d...\n", args.ip_address, args.port);
            printf("(Multiplayer implementation coming in Phase 5)\n");
            break;

        case MODE_SPECTATOR:
            printf("=== GOMOKU GAME ===\n");
            printf("Spectating game at %s:%d...\n", args.ip_address, args.port);
            printf("(Spectator implementation coming in Phase 7)\n");
            break;
    }

    printf("\nPress Enter to exit...\n");
    getchar();

    return 0;
}
