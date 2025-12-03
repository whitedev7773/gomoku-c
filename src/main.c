#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <string.h>
#include "utils/terminal_check.h"
#include "utils/arg_parser.h"
#include "ui/menu_ui.h"
#include "ui/theme.h"
#include "ui/theme_selector_ui.h"
#include "game/singleplay.h"
#include "game/multiplayer.h"
#include "game/spectator.h"
#include "game/replay.h"
#include "game/ai_engine.h"
#include "network/network.h"
#include <ncurses.h>

int main(int argc, char *argv[])
{
    // Parse command line arguments
    ParsedArgs args = parse_arguments(argc, argv);

    if (!args.valid)
    {
        if (args.error_message[0] != '\0')
        {
            printf("%s\n", args.error_message);
            display_usage(argv[0]);
        }
        return 1;
    }

    // Check terminal size
    TerminalSize current_size = get_terminal_size();

    while (!check_terminal_size())
    {
        system("clear");
        display_terminal_size_error(current_size);
        printf("Press Ctrl+C to exit or resize terminal and press Enter to continue...\n");
        getchar();
        current_size = get_terminal_size();
    }

    system("clear");

    // Handle different game modes
    switch (args.mode)
    {
    case MODE_MENU:
    {
        setlocale(LC_ALL, "");
        initscr();
        cbreak();
        noecho();
        curs_set(0);

        // Limit stdscr to 100x30
        wresize(stdscr, 30, 100);

        // 테마 초기화
        theme_init(THEME_WHITE);

        // Always use fixed 100x30 size
        WINDOW *menu_win = newwin(30, 100, 0, 0);
        keypad(menu_win, TRUE);

        MenuUI menu;
        menu_ui_init(&menu);

        bool running = true;
        MenuOption selected_option = MENU_SINGLEPLAY;

        while (running)
        {
            menu_ui_render(menu_win, &menu);

            int ch = wgetch(menu_win);

            switch (ch)
            {
            case KEY_UP:
                menu_ui_move_selection(&menu, -1);
                break;
            case KEY_DOWN:
                menu_ui_move_selection(&menu, 1);
                break;
            case '\n':
            case KEY_ENTER:
                selected_option = menu_ui_get_selected(&menu);
                running = false;
                break;
            case 'q':
            case 'Q':
                running = false;
                break;
            }
        }

        delwin(menu_win);
        endwin();

        system("clear");
        if (selected_option == MENU_SINGLEPLAY)
        {
            // 난이도 선택
            printf("=== GOMOKU - SINGLEPLAY ===\n");
            printf("Select difficulty:\n");
            printf("  1. Easy\n");
            printf("  2. Hard\n");
            printf("Enter your choice (1-2): ");

            char choice;
            scanf(" %c", &choice);

            if (choice == '1') {
                singleplay_run(AI_EASY);
            } else if (choice == '2') {
                singleplay_run(AI_HARD);
            } else {
                printf("Invalid choice. Returning to menu.\n");
            }
        }
        else if (selected_option == MENU_MULTIPLAY)
        {
            // 호스트/클라이언트 선택
            printf("=== GOMOKU - MULTIPLAY ===\n");
            printf("Select mode:\n");
            printf("  1. Host (create game)\n");
            printf("  2. Join (connect to game)\n");
            printf("Enter your choice (1-2): ");

            char choice;
            scanf(" %c", &choice);
            getchar();  // 버퍼 비우기

            if (choice == '1') {
                multiplayer_run_host(DEFAULT_PORT);
            } else if (choice == '2') {
                char server_ip[64];
                int port;
                printf("Enter server IP address: ");
                scanf("%s", server_ip);
                printf("Enter port (default 7773): ");
                if (scanf("%d", &port) != 1) {
                    port = DEFAULT_PORT;
                }
                getchar();  // 버퍼 비우기

                multiplayer_run_client(server_ip, port);
            } else {
                printf("Invalid choice. Returning to menu.\n");
            }
        }
        else if (selected_option == MENU_SPECTATOR)
        {
            printf("=== GOMOKU - SPECTATOR ===\n");
            char server_ip[64];
            int port;
            char spectator_name[MAX_PLAYER_NAME + 1];

            printf("Enter server IP address: ");
            scanf("%s", server_ip);
            printf("Enter port (default 7773): ");
            if (scanf("%d", &port) != 1) {
                port = DEFAULT_PORT;
            }
            getchar();  // 버퍼 비우기

            printf("Enter your name (max 8 chars): ");
            fgets(spectator_name, sizeof(spectator_name), stdin);
            spectator_name[strcspn(spectator_name, "\n")] = '\0';
            if (strlen(spectator_name) == 0) {
                strcpy(spectator_name, "Viewer");
            }

            spectator_run(server_ip, port, spectator_name);
        }
        else if (selected_option == MENU_REPLAY)
        {
            replay_run_with_selection();
        }
        else if (selected_option == MENU_THEME)
        {
            // 테마 선택 화면 실행
            theme_selector_run();

            // 메뉴로 돌아와서 테마 다시 초기화
            theme_init(theme_get_current());
        }
        else if (selected_option == MENU_EXIT)
        {
            // 프로그램 종료
            printf("Exiting...\n");
        }
        break;
    }

    case MODE_SINGLEPLAY_EASY:
        singleplay_run(AI_EASY);
        break;

    case MODE_SINGLEPLAY_HARD:
        singleplay_run(AI_HARD);
        break;

    case MODE_MULTIPLAY_HOST:
        multiplayer_run_host(args.port > 0 ? args.port : DEFAULT_PORT);
        break;

    case MODE_MULTIPLAY_CLIENT:
        multiplayer_run_client(args.ip_address, args.port);
        break;

    case MODE_SPECTATOR:
        spectator_run(args.ip_address, args.port, "Viewer");
        break;
    }

    printf("\nPress Enter to exit...\n");
    getchar();

    return 0;
}
