#include "singleplay.h"
#include "board.h"
#include "game_logic.h"
#include "turn_manager.h"
#include "game_logger.h"
#include "../ui/ui_manager.h"
#include "../ui/board_ui.h"
#include "../ui/input_handler.h"
#include "../ui/game_info_ui.h"
#include "../ui/log_ui.h"
#include "../ui/theme.h"
#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <string.h>

int singleplay_run(AIDifficulty difficulty) {
    // Locale 설정 (UTF-8 지원)
    setlocale(LC_ALL, "");

    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    // stdscr을 100x30으로 고정
    wresize(stdscr, 30, 100);

    // 테마 초기화
    theme_init(theme_get_current());

    // UI Manager 초기화
    UIManager ui_mgr;
    if (!ui_manager_init(&ui_mgr)) {
        endwin();
        printf("Failed to initialize UI manager\n");
        return -1;
    }

    // 게임 컴포넌트 초기화
    Board board;
    board_init(&board);

    BoardCursor cursor;
    board_ui_init_cursor(&cursor);

    TurnManager turn_mgr;
    turn_manager_init(&turn_mgr, BLACK); // 유저가 선공 (BLACK)

    AIEngine ai;
    ai_init(&ai, difficulty, WHITE); // AI는 WHITE

    GameInfoUI info_ui;
    game_info_ui_init(&info_ui);

    LogUI log_ui;
    log_ui_init(&log_ui);

    GameLogger logger;
    // 로거 초기화 및 에러 체크
    if (!logger_init(&logger)) {
        log_ui_add_message(&log_ui, "Warning: Failed to create log file");
        log_ui_add_message(&log_ui, "Game will continue without logging");
    }

    // 시작 메시지
    char start_msg[128];
    snprintf(start_msg, sizeof(start_msg), "Game started! You: BLACK, AI: WHITE (%s)",
             difficulty == AI_EASY ? "Easy" : "Hard");
    log_ui_add_message(&log_ui, start_msg);

    // 메인 게임 루프
    bool game_running = true;
    bool game_over = false;
    GameResult result = GAME_ONGOING;

    // 보드 윈도우에서 키 입력 받기
    keypad(ui_mgr.board_win, TRUE);
    wtimeout(ui_mgr.board_win, 100); // 100ms timeout (AI 턴 처리 및 타이머 업데이트용)

    while (game_running) {
        // UI 렌더링
        ui_manager_clear_all(&ui_mgr);

        // 보드 렌더링
        board_ui_render(ui_mgr.board_win, &board, &cursor);

        // 게임 정보 렌더링 (하단)
        game_info_ui_render_bottom(ui_mgr.bottom_win, &board, &turn_mgr, &info_ui);

        // 로그 렌더링 (우측 채팅창 영역)
        mvwprintw(ui_mgr.chat_win, 0, 0, "=== System Log ===");
        log_ui_render(ui_mgr.chat_win, &log_ui, 2, 1);

        // 우측 info 창 (현재 턴 표시)
        Stone current_player = turn_manager_get_current_player(&turn_mgr);
        mvwprintw(ui_mgr.info_win, 0, 0, "Current: %s",
                  current_player == BLACK ? "You (BLACK)" : "AI (WHITE)");

        ui_manager_refresh_all(&ui_mgr);

        // 게임 종료 체크
        if (!game_over) {
            result = game_check_winner(&board);
            if (result != GAME_ONGOING) {
                game_over = true;

                char result_msg[128];
                if (result == GAME_BLACK_WIN) {
                    snprintf(result_msg, sizeof(result_msg), "You WIN! Congratulations!");
                } else if (result == GAME_WHITE_WIN) {
                    snprintf(result_msg, sizeof(result_msg), "AI WINS! Better luck next time.");
                } else {
                    snprintf(result_msg, sizeof(result_msg), "Game ended in a DRAW!");
                }
                log_ui_add_message(&log_ui, result_msg);
                log_ui_add_message(&log_ui, "Press 'q' to quit");

                logger_close(&logger);
                continue;
            }
        }

        // AI 턴 처리
        if (!game_over && current_player == WHITE) {
            // AI가 수를 계산
            Position ai_move;
            if (ai_get_next_move(&ai, &board, &ai_move)) {
                if (board_place_stone(&board, ai_move.row, ai_move.col, WHITE)) {
                    // 로그 기록
                    char move_msg[128];
                    snprintf(move_msg, sizeof(move_msg), "AI placed at %c%02d",
                             board_ui_col_to_char(ai_move.col), ai_move.row + 1);
                    log_ui_add_message(&log_ui, move_msg);

                    logger_log_move(&logger, WHITE, ai_move.row, ai_move.col,
                                   board_get_move_count(&board));

                    // 턴 변경
                    turn_manager_next_turn(&turn_mgr);
                }
            }

            // AI 턴 후 약간의 지연 (보기 좋게)
            usleep(300000); // 300ms
            continue;
        }

        // 유저 입력 처리 (BLACK 턴일 때만)
        if (!game_over && current_player == BLACK) {
            InputEvent event = input_get_event(ui_mgr.board_win);

            if (event.action != INPUT_NONE) {
                switch (event.action) {
                    case INPUT_MOVE_UP:
                        board_ui_move_cursor(&cursor, -1, 0);
                        break;
                    case INPUT_MOVE_DOWN:
                        board_ui_move_cursor(&cursor, 1, 0);
                        break;
                    case INPUT_MOVE_LEFT:
                        board_ui_move_cursor(&cursor, 0, -1);
                        break;
                    case INPUT_MOVE_RIGHT:
                        board_ui_move_cursor(&cursor, 0, 1);
                        break;
                    case INPUT_PLACE_STONE:
                        if (board_is_empty(&board, cursor.cursor_row, cursor.cursor_col)) {
                            if (board_place_stone(&board, cursor.cursor_row, cursor.cursor_col, BLACK)) {
                                char move_msg[128];
                                snprintf(move_msg, sizeof(move_msg), "You placed at %c%02d",
                                        board_ui_col_to_char(cursor.cursor_col), cursor.cursor_row + 1);
                                log_ui_add_message(&log_ui, move_msg);

                                logger_log_move(&logger, BLACK, cursor.cursor_row, cursor.cursor_col,
                                               board_get_move_count(&board));

                                turn_manager_next_turn(&turn_mgr);
                            }
                        } else {
                            log_ui_add_message(&log_ui, "Position already occupied!");
                        }
                        break;
                    case INPUT_QUIT:
                        game_running = false;
                        break;
                    case INPUT_RESIGN:
                        log_ui_add_message(&log_ui, "You resigned. AI WINS!");
                        log_ui_add_message(&log_ui, "Press 'q' to quit");
                        game_over = true;
                        break;
                    default:
                        break;
                }
            }
        }

        // 게임 종료 후 q로만 나갈 수 있음
        if (game_over) {
            int ch = wgetch(ui_mgr.board_win);
            if (ch == 'q' || ch == 'Q') {
                game_running = false;
            }
        }
    }

    // 정리
    logger_close(&logger);
    ui_manager_cleanup(&ui_mgr);
    endwin();

    return 0;
}
