#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <string.h>
#include "utils/terminal_check.h"
#include "utils/arg_parser.h"
#include "ui/menu/menu_ui.h"
#include "ui/core/theme.h"
#include "ui/menu/theme_selector_ui.h"
#include "ui/core/input_handler.h"
#include "game/mode/singleplay/singleplay.h"
#include "game/mode/multiplay/multiplayer.h"
#include "game/mode/multiplay/spectator.h"
#include "game/feature/replay.h"
#include "game/ai/ai_engine.h"
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
        bool app_running = true; // 앱 전체 루프

        while (app_running)
        {
            setlocale(LC_ALL, "");
            initscr();
            cbreak();
            noecho();
            curs_set(0);

            // Limit stdscr to 100x30
            wresize(stdscr, 30, 100);

            // 저장된 테마 불러오기 및 초기화
            ThemeType saved_theme = theme_load_from_config();
            theme_init(saved_theme);

            // Always use fixed 100x30 size
            WINDOW *menu_win = newwin(30, 100, 0, 0);
            keypad(menu_win, TRUE);

            // 게임패드 입력 핸들러 초기화
            InputHandler input_handler;
            input_handler_init(&input_handler);

            MenuUI menu;
            menu_ui_init(&menu);

            bool running = true;
            MenuOption selected_option = MENU_SINGLEPLAY;
            bool first_render = true; // 첫 렌더링 여부

            while (running)
            {
                if (first_render)
                {
                    menu_ui_render(menu_win, &menu);
                    first_render = false;
                }
                else
                {
                    menu_ui_render_options_only(menu_win, &menu);
                }

                InputEvent event = input_handler_get_event(&input_handler, menu_win);

                switch (event.action)
                {
                case INPUT_MOVE_UP:
                    menu_ui_move_selection(&menu, -1);
                    break;
                case INPUT_MOVE_DOWN:
                    menu_ui_move_selection(&menu, 1);
                    break;
                case INPUT_MOVE_LEFT:
                    menu_ui_change_page(&menu, -1);
                    break;
                case INPUT_MOVE_RIGHT:
                    menu_ui_change_page(&menu, 1);
                    break;
                case INPUT_PLACE_STONE:
                    selected_option = menu_ui_get_selected(&menu);

                    // 테마 선택은 메뉴 루프 내에서 처리
                    if (selected_option == MENU_THEME)
                    {
                        input_handler_cleanup(&input_handler);
                        delwin(menu_win);
                        endwin();

                        // 테마 선택 화면 실행
                        theme_selector_run();

                        // 메뉴로 돌아오기
                        initscr();
                        cbreak();
                        noecho();
                        curs_set(0);
                        wresize(stdscr, 30, 100);
                        theme_init(theme_get_current());

                        menu_win = newwin(30, 100, 0, 0);
                        keypad(menu_win, TRUE);

                        // 게임패드 다시 초기화
                        input_handler_init(&input_handler);

                        // 전체 재렌더링 필요
                        first_render = true;

                        // 루프 계속
                        continue;
                    }
                    else
                    {
                        running = false;
                    }
                    break;
                case INPUT_QUIT:
                    // 메인 메뉴에서는 ESC/Q로 종료하지 않음
                    break;
                default:
                    break;
                }
            }

            input_handler_cleanup(&input_handler);
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

                char difficulty_choice;
                scanf(" %c", &difficulty_choice);

                // 규칙 선택
                printf("\nSelect game rule:\n");
                printf("  1. Standard (No forbidden moves)\n");
                printf("  2. Renju (Forbidden moves for BLACK)\n");
                printf("Enter your choice (1-2): ");

                char rule_choice;
                scanf(" %c", &rule_choice);

                GameRule rule = (rule_choice == '1') ? RULE_STANDARD : RULE_RENJU;

                if (difficulty_choice == '1')
                {
                    singleplay_run(AI_EASY, rule);
                }
                else if (difficulty_choice == '2')
                {
                    singleplay_run(AI_HARD, rule);
                }
                else
                {
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

                char mode_choice;
                scanf(" %c", &mode_choice);
                getchar(); // 버퍼 비우기

                if (mode_choice == '1')
                {
                    // 호스트: 규칙 선택
                    printf("\nSelect game rule:\n");
                    printf("  1. Standard (No forbidden moves)\n");
                    printf("  2. Renju (Forbidden moves for BLACK)\n");
                    printf("Enter your choice (1-2): ");

                    char rule_choice;
                    scanf(" %c", &rule_choice);
                    getchar(); // 버퍼 비우기

                    GameRule rule = (rule_choice == '1') ? RULE_STANDARD : RULE_RENJU;
                    multiplayer_run_host(DEFAULT_PORT, rule);
                }
                else if (mode_choice == '2')
                {
                    // 클라이언트: 규칙은 호스트로부터 받음
                    char server_ip[64];
                    int port;
                    printf("Enter server IP address: ");
                    scanf("%s", server_ip);
                    printf("Enter port (default 7773): ");
                    if (scanf("%d", &port) != 1)
                    {
                        port = DEFAULT_PORT;
                    }
                    getchar(); // 버퍼 비우기

                    multiplayer_run_client(server_ip, port, RULE_STANDARD); // 규칙은 호스트로부터 받을 예정
                }
                else
                {
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
                if (scanf("%d", &port) != 1)
                {
                    port = DEFAULT_PORT;
                }
                getchar(); // 버퍼 비우기

                printf("Enter your name (max 8 chars): ");
                fgets(spectator_name, sizeof(spectator_name), stdin);
                spectator_name[strcspn(spectator_name, "\n")] = '\0';
                if (strlen(spectator_name) == 0)
                {
                    strcpy(spectator_name, "Viewer");
                }

                spectator_run(server_ip, port, spectator_name);
            }
            else if (selected_option == MENU_REPLAY)
            {
                replay_run_with_selection();
                // 리플레이 종료 후 메인 메뉴로 돌아감
                continue; // app_running 루프 계속
            }
            else if (selected_option == MENU_EXIT)
            {
                // 프로그램 종료
                app_running = false;
                printf("Exiting...\n");
            }
        } // end of app_running while loop
        break;
    }

    case MODE_SINGLEPLAY_EASY:
        singleplay_run(AI_EASY, RULE_RENJU); // CLI 모드는 기본 Renju Rule
        break;

    case MODE_SINGLEPLAY_HARD:
        singleplay_run(AI_HARD, RULE_RENJU); // CLI 모드는 기본 Renju Rule
        break;

    case MODE_MULTIPLAY_HOST:
        multiplayer_run_host(args.port > 0 ? args.port : DEFAULT_PORT, RULE_RENJU); // CLI 모드는 기본 Renju Rule
        break;

    case MODE_MULTIPLAY_CLIENT:
        multiplayer_run_client(args.ip_address, args.port, RULE_STANDARD); // 규칙은 호스트로부터 받음
        break;

    case MODE_SPECTATOR:
        spectator_run(args.ip_address, args.port, "Viewer");
        break;
    }

    printf("\nPress Enter to exit...\n");
    getchar();

    return 0;
}
