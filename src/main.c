#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <string.h>
#include <signal.h>
#include "utils/terminal_check.h"
#include "utils/arg_parser.h"
#include "ui/menu/menu_ui.h"
#include "ui/core/theme.h"
#include "ui/core/ui_manager.h"
#include "ui/menu/theme_selector_ui.h"
#include "ui/menu/singleplay_menu_ui.h"
#include "ui/menu/multiplay_menu_ui.h"
#include "ui/core/input_handler.h"
#include "game/mode/singleplay/singleplay.h"
#include "game/mode/multiplay/multiplayer.h"
#include "game/mode/multiplay/spectator.h"
#include "game/feature/replay.h"
#include "game/ai/ai_engine.h"
#include "network/core/network.h"
#include <ncurses.h>

// 터미널 리사이징 감지를 위한 시그널 핸들러
static volatile sig_atomic_t terminal_resized = 0;

void handle_sigwinch(int sig)
{
    terminal_resized = 1;
}

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

    // 터미널 리사이징 감지를 위한 시그널 핸들러 설정
    signal(SIGWINCH, handle_sigwinch);

    // Check terminal size
    TerminalSize current_size = get_terminal_size();

    while (!check_terminal_size())
    {
        if (terminal_resized)
        {
            // 화면 지우기 (system("clear") 대신 ANSI escape codes 사용)
            printf("\033[2J\033[H");
            terminal_resized = 0;
        }

        display_terminal_size_error(current_size);
        printf("Press Ctrl+C to exit or resize terminal and wait for update...\n");

        // 터미널 크기 갱신
        current_size = get_terminal_size();

        // 시그널이 올 때까지 대기 (실시간 반응)
        pause();
    }

    // system("clear");

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

            // Limit stdscr to UI_MIN_WIDTH x UI_MIN_HEIGHT
            wresize(stdscr, UI_MIN_HEIGHT, UI_MIN_WIDTH);

            // 저장된 테마 불러오기 및 초기화
            ThemeType saved_theme = theme_load_from_config();
            theme_init(saved_theme);

            // Always use fixed UI_MIN_WIDTH x UI_MIN_HEIGHT size
            WINDOW *menu_win = newwin(UI_MIN_HEIGHT, UI_MIN_WIDTH, 0, 0);
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
                        wresize(stdscr, UI_MIN_HEIGHT, UI_MIN_WIDTH);
                        theme_init(theme_get_current());

                        menu_win = newwin(UI_MIN_HEIGHT, UI_MIN_WIDTH, 0, 0);
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

            if (selected_option == MENU_SINGLEPLAY)
            {
                // TUI 기반 난이도 및 규칙 선택
                AIDifficulty difficulty;
                GameRule rule;

                if (singleplay_run_settings(&difficulty, &rule) == 0)
                {
                    singleplay_run(difficulty, rule);
                }
                // 취소 시 메인 메뉴로 돌아감
            }
            else if (selected_option == MENU_MULTIPLAY)
            {
                // TUI 기반 HOST/JOIN 선택
                int mode_result = multiplay_select_mode();

                if (mode_result == 0)
                {
                    // HOST 모드 선택됨 - 닉네임 입력
                    char host_name[MAX_PLAYER_NAME + 1];
                    if (multiplay_input_host_settings(host_name) == 0)
                    {
                        // 규칙 선택
                        int rule_result = multiplay_select_rule();
                        if (rule_result >= 0)
                        {
                            GameRule rule = (rule_result == 0) ? RULE_STANDARD : RULE_RENJU;
                            // 바로 호스트 실행 (연결 대기는 multiplayer_run_host 내부에서 처리)
                            multiplayer_run_host(DEFAULT_PORT, rule, host_name);
                            // 취소 시 메인 메뉴로 돌아감
                        }
                    }
                    // 취소 시 메인 메뉴로 돌아감
                }
                else if (mode_result == 1)
                {
                    // JOIN 모드 선택됨 - IP/PORT/닉네임 입력
                    char server_ip[64];
                    int port;
                    char player_name[MAX_PLAYER_NAME + 1];

                    if (multiplay_input_connection(server_ip, &port, player_name) == 0)
                    {
                        // 바로 클라이언트 실행 (연결은 multiplayer_run_client 내부에서 처리)
                        multiplayer_run_client(server_ip, port, RULE_STANDARD, player_name);
                        // 연결 실패 또는 취소 시 메인 메뉴로 돌아감
                    }
                    // 취소 시 메인 메뉴로 돌아감
                }
                // mode_result == -1 이면 취소 (메인 메뉴로 돌아감)
            }
            else if (selected_option == MENU_SPECTATOR)
            {
                // TUI 기반 관전자 연결 정보 입력
                char server_ip[64];
                int port;
                char spectator_name[MAX_PLAYER_NAME + 1];

                if (spectator_input_connection(server_ip, &port, spectator_name) == 0)
                {
                    // 연결 대기 화면 표시 (10초 타임아웃)
                    if (spectator_wait_connection(server_ip, port, spectator_name) == 0)
                    {
                        spectator_run(server_ip, port, spectator_name);
                    }
                    // 연결 실패 또는 취소 시 메인 메뉴로 돌아감
                }
                // 취소 시 메인 메뉴로 돌아감
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
                // printf("Exiting...\n");
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
        multiplayer_run_host(args.port > 0 ? args.port : DEFAULT_PORT, RULE_RENJU, "Host"); // CLI 모드는 기본 Renju Rule
        break;

    case MODE_MULTIPLAY_CLIENT:
        multiplayer_run_client(args.ip_address, args.port, RULE_STANDARD, "Client"); // 규칙은 호스트로부터 받음
        break;

    case MODE_SPECTATOR:
        spectator_run(args.ip_address, args.port, "Viewer");
        break;
    }

    // printf("\nPress Enter to exit...\n");
    // getchar();

    return 0;
}
