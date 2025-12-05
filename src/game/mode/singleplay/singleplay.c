#include "singleplay.h"
#include "../../core/board.h"
#include "../../core/game_logic.h"
#include "../../core/turn_manager.h"
#include "../../feature/game_logger.h"
#include "../../../ui/core/ui_manager.h"
#include "../../../ui/game/board/board_ui.h"
#include "../../../ui/core/input_handler.h"
#include "../../../ui/game/game_info_ui.h"
#include "../../../ui/game/log/log_ui.h"
#include "../../../ui/menu/modal_ui.h"
#include "../../../ui/core/theme.h"
#include "../../../ui/game/border/ingame_border.h"
#include "../../../utils/terminal_check.h"
#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <string.h>

int singleplay_run(AIDifficulty difficulty, GameRule rule)
{
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
    if (!ui_manager_init(&ui_mgr))
    {
        endwin();
        printf("Failed to initialize UI manager\n");
        return -1;
    }

    // 인게임 Border 그리기
    ingame_border_draw();
    refresh();

    // 게임 컴포넌트 초기화
    Board board;
    board_init_with_rule(&board, rule);

    BoardCursor cursor;
    board_init_cursor(&cursor);

    TurnManager turn_mgr;
    turn_manager_init(&turn_mgr, BLACK); // 유저가 선공 (BLACK)

    AIEngine ai;
    ai_init(&ai, difficulty, WHITE); // AI는 WHITE

    GameInfoUI info_ui;
    game_info_ui_init(&info_ui);

    LogUI log_ui;
    log_init(&log_ui);

    ModalUI modal_ui;
    modal_ui_init(&modal_ui);

    GameLogger logger;
    // 로거 초기화 및 에러 체크
    if (!logger_init(&logger))
    {
        log_add_msg(&log_ui, "Warning: Failed to create log file");
        log_add_msg(&log_ui, "Game will continue without logging");
    }

    // 게임패드 입력 핸들러 초기화
    InputHandler input_handler;
    input_handler_init(&input_handler);

    // 시작 메시지
    char start_msg[128];
    snprintf(start_msg, sizeof(start_msg), "Game started! You: BLACK, AI: WHITE (%s)",
             difficulty == AI_EASY ? "Easy" : "Hard");
    log_add_msg(&log_ui, start_msg);

    // 메인 게임 루프
    bool game_running = true;
    bool game_over = false;
    GameResult result = GAME_ONGOING;

    // Dirty flag 기반 렌더링을 위한 변수
    bool first_render = true;
    UIRenderFlags *render_flags = &ui_mgr.render_flags;

    // 터미널 크기 경고 상태
    bool terminal_warning_shown = false;
    TerminalSize prev_term_size = get_terminal_size(); // 이전 터미널 크기

    // 보드 윈도우에서 키 입력 받기
    keypad(ui_mgr.board_win, TRUE);
    wtimeout(ui_mgr.board_win, 100); // 100ms timeout (AI 턴 처리 및 타이머 업데이트용)

    while (game_running)
    {
        // ============================================
        // 터미널 크기 체크 (싱글플레이: 일시정지)
        // ============================================
        TerminalSizeStatus term_status = check_terminal_size_ingame();
        TerminalSize cur_size = get_terminal_size();

        // 터미널 크기가 변했고 작은 상태일 때만 화면 clear (잔상 제거)
        bool size_changed = (cur_size.width != prev_term_size.width || cur_size.height != prev_term_size.height);
        if (size_changed)
        {
            prev_term_size = cur_size;
            if (term_status == TERMINAL_SIZE_TOO_SMALL)
            {
                clear();
                refresh();
            }
        }

        if (term_status == TERMINAL_SIZE_TOO_SMALL && !terminal_warning_shown)
        {
            // 터미널이 작아짐 - 경고 모달 표시 및 일시정지
            char warn_msg[MODAL_MAX_MESSAGE_LENGTH];
            snprintf(warn_msg, sizeof(warn_msg),
                     "Terminal too small! (%dx%d)\nRequired: %dx%d\nResize to continue.",
                     cur_size.width, cur_size.height,
                     MIN_TERMINAL_WIDTH, MIN_TERMINAL_HEIGHT);
            modal_ui_show(&modal_ui, MODAL_TERMINAL_WARNING, warn_msg);
            terminal_warning_shown = true;

            // 타이머 일시정지
            turn_manager_pause(&turn_mgr);

            // 화면 지우기
            clear();
            refresh();
        }
        else if (term_status == TERMINAL_SIZE_RESTORED && terminal_warning_shown)
        {
            // 터미널 크기 복원됨 - 모달 닫기 및 재개
            modal_ui_close(&modal_ui);
            terminal_warning_shown = false;

            // 타이머 재개
            turn_manager_resume(&turn_mgr);

            // 전체 화면 다시 그리기
            clear();
            refresh();
            ingame_border_draw();
            refresh();
            first_render = true;
        }

        // 터미널 경고 모달이 표시 중이면 모달만 렌더링
        if (terminal_warning_shown)
        {
            // 터미널 크기 변경 시 모달 메시지 업데이트
            char warn_msg[MODAL_MAX_MESSAGE_LENGTH];
            snprintf(warn_msg, sizeof(warn_msg),
                     "Terminal too small! (%dx%d)\nRequired: %dx%d\nResize to continue.",
                     cur_size.width, cur_size.height,
                     MIN_TERMINAL_WIDTH, MIN_TERMINAL_HEIGHT);
            strncpy(modal_ui.message, warn_msg, MODAL_MAX_MESSAGE_LENGTH - 1);

            // 모달 렌더링
            modal_ui_render(stdscr, &modal_ui);

            // 터미널 크기가 복원될 때까지 대기
            usleep(100000); // 100ms
            continue;
        }

        // 현재 플레이어에 따라 금수 마크 업데이트 (Renju Rule)
        Stone current_player = turn_manager_get_current_player(&turn_mgr);
        if (current_player == BLACK)
        {
            board_update_forbidden_marks(&board, BLACK);
        }

        // ============================================
        // 선택적 UI 렌더링 (Dirty Flag 기반)
        // ============================================

        // 타이머와 플레이 시간은 항상 dirty로 설정 (매 루프 체크)
        ui_render_flags_set(render_flags, RENDER_TIMER);
        ui_render_flags_set(render_flags, RENDER_PLAY_TIME);

        // 보드 렌더링
        board_render(ui_mgr.board_win, &board, &cursor,
                     render_flags, first_render);

        // 게임 정보 렌더링 (하단)
        game_info_ui_selective_render(ui_mgr.bottom_win, &board, &turn_mgr,
                                      &info_ui, render_flags, first_render);

        // 로그 렌더링 (우측 채팅창 영역)
        if (first_render)
        {
            mvwprintw(ui_mgr.chat_win, 0, 0, "=== System Log ===");
            wrefresh(ui_mgr.chat_win);
        }
        log_render_sel(ui_mgr.chat_win, &log_ui, render_flags, first_render, 2, 1);

        // 우측 info 창 (현재 턴 표시) - 턴 변경 시에만
        if (first_render || ui_render_flags_is_set(render_flags, RENDER_INFO))
        {
            mvwprintw(ui_mgr.info_win, 0, 0, "Current: %s",
                      current_player == BLACK ? "You (BLACK)" : "AI (WHITE)");
            wrefresh(ui_mgr.info_win);
            ui_render_flags_clear(render_flags, RENDER_INFO);
        }

        // 첫 렌더링 완료
        first_render = false;

        // 게임 종료 체크
        if (!game_over)
        {
            result = game_check_winner(&board);
            if (result != GAME_ONGOING)
            {
                game_over = true;

                char result_msg[128];
                if (result == GAME_BLACK_WIN)
                {
                    snprintf(result_msg, sizeof(result_msg), "You WIN! Congratulations!");
                }
                else if (result == GAME_WHITE_WIN)
                {
                    snprintf(result_msg, sizeof(result_msg), "AI WINS! Better luck next time.");
                }
                else
                {
                    snprintf(result_msg, sizeof(result_msg), "Game ended in a DRAW!");
                }
                log_add_msg(&log_ui, result_msg);

                // 모달 표시
                modal_ui_show(&modal_ui, MODAL_GAME_RESULT, result_msg);

                logger_close(&logger);
                continue;
            }
        }

        // 유저 턴 타임아웃 체크 (BLACK 턴일 때만)
        if (!game_over && current_player == BLACK && turn_manager_is_timeout(&turn_mgr))
        {
            game_over = true;
            log_add_msg(&log_ui, "Time's up! You LOSE!");
            modal_ui_show(&modal_ui, MODAL_GAME_RESULT, "Time's up! You LOSE!");
            logger_close(&logger);
            continue;
        }

        // AI 턴 처리
        if (!game_over && current_player == WHITE)
        {
            // AI가 수를 계산
            Position ai_move;
            if (ai_get_next_move(&ai, &board, &ai_move))
            {
                // 이전 마지막 수 위치 저장 (강조 제거용)
                int prev_last_row = board.last_row;
                int prev_last_col = board.last_col;

                if (board_place_stone(&board, ai_move.row, ai_move.col, WHITE))
                {
                    // 로그 기록
                    char move_msg[128];
                    snprintf(move_msg, sizeof(move_msg), "AI placed at %c%02d",
                             board_col_to_char(ai_move.col), ai_move.row + 1);
                    log_add_msg(&log_ui, move_msg);

                    logger_log_move(&logger, WHITE, ai_move.row, ai_move.col,
                                    board_get_move_count(&board));

                    // 이전 마지막 수 위치도 dirty로 마킹 (강조 제거)
                    if (prev_last_row >= 0 && prev_last_col >= 0)
                    {
                        ui_render_flags_add_dirty_cell(render_flags, prev_last_row, prev_last_col);
                    }
                    // 돌이 놓인 셀을 dirty로 마킹
                    ui_render_flags_add_dirty_cell(render_flags, ai_move.row, ai_move.col);
                    ui_render_flags_set(render_flags, RENDER_LAST_MOVE);
                    ui_render_flags_set(render_flags, RENDER_CURRENT_TURN);
                    ui_render_flags_set(render_flags, RENDER_INFO);

                    // 턴 변경
                    turn_manager_next_turn(&turn_mgr);
                }
            }

            // AI 턴 후 약간의 지연 (보기 좋게)
            usleep(300000); // 300ms
            continue;
        }

        // 유저 입력 처리 (BLACK 턴일 때만)
        if (!game_over && current_player == BLACK)
        {
            InputEvent event = input_handler_get_event(&input_handler, ui_mgr.board_win);

            if (event.action != INPUT_NONE)
            {
                switch (event.action)
                {
                case INPUT_MOVE_UP:
                    board_move_cursor_f(&cursor, -1, 0, render_flags);
                    break;
                case INPUT_MOVE_DOWN:
                    board_move_cursor_f(&cursor, 1, 0, render_flags);
                    break;
                case INPUT_MOVE_LEFT:
                    board_move_cursor_f(&cursor, 0, -1, render_flags);
                    break;
                case INPUT_MOVE_RIGHT:
                    board_move_cursor_f(&cursor, 0, 1, render_flags);
                    break;
                case INPUT_PLACE_STONE:
                    if (board_is_empty(&board, cursor.cursor_row, cursor.cursor_col))
                    {
                        // Renju Rule: 흑돌은 금수 위치에 놓을 수 없음
                        if (board_is_forbidden(&board, cursor.cursor_row, cursor.cursor_col))
                        {
                            log_add_msg(&log_ui, "Forbidden move! (Renju Rule)");
                        }
                        else
                        {
                            // 이전 마지막 수 위치 저장 (강조 제거용)
                            int prev_last_row = board.last_row;
                            int prev_last_col = board.last_col;

                            if (board_place_stone(&board, cursor.cursor_row, cursor.cursor_col, BLACK))
                            {
                                char move_msg[128];
                                snprintf(move_msg, sizeof(move_msg), "You placed at %c%02d",
                                         board_col_to_char(cursor.cursor_col), cursor.cursor_row + 1);
                                log_add_msg(&log_ui, move_msg);

                                logger_log_move(&logger, BLACK, cursor.cursor_row, cursor.cursor_col,
                                                board_get_move_count(&board));

                                // 이전 마지막 수 위치도 dirty로 마킹 (강조 제거)
                                if (prev_last_row >= 0 && prev_last_col >= 0)
                                {
                                    ui_render_flags_add_dirty_cell(render_flags, prev_last_row, prev_last_col);
                                }
                                // 돌이 놓인 셀을 dirty로 마킹
                                ui_render_flags_add_dirty_cell(render_flags, cursor.cursor_row, cursor.cursor_col);
                                ui_render_flags_set(render_flags, RENDER_LAST_MOVE);
                                ui_render_flags_set(render_flags, RENDER_CURRENT_TURN);
                                ui_render_flags_set(render_flags, RENDER_INFO);

                                turn_manager_next_turn(&turn_mgr);
                            }
                        }
                    }
                    else
                    {
                        log_add_msg(&log_ui, "Position already occupied!");
                    }
                    break;
                case INPUT_QUIT:
                    game_running = false;
                    break;
                case INPUT_RESIGN:
                    log_add_msg(&log_ui, "You resigned. AI WINS!");
                    modal_ui_show(&modal_ui, MODAL_GAME_RESULT, "You resigned. AI WINS!");
                    game_over = true;
                    break;
                default:
                    break;
                }
            }
        }

        // 게임 종료 후 모달 처리
        if (game_over)
        {
            // 모달 렌더링
            if (modal_ui_is_active(&modal_ui))
            {
                modal_ui_render(stdscr, &modal_ui);
            }

            InputEvent event = input_handler_get_event(&input_handler, ui_mgr.board_win);

            // 모달이 활성화되어 있으면 모달 입력 처리
            if (modal_ui_is_active(&modal_ui) && event.action != INPUT_NONE)
            {
                ModalResult result = modal_ui_handle_action(&modal_ui, event.action);
                if (result == MODAL_RESULT_OK || result == MODAL_RESULT_CANCEL)
                {
                    modal_ui_close(&modal_ui);
                    game_running = false; // 메인 화면으로 이동
                }
            }
            else if (event.action == INPUT_QUIT)
            {
                game_running = false;
            }
        }
    }

    // 정리
    input_handler_cleanup(&input_handler);
    logger_close(&logger);
    ui_manager_cleanup(&ui_mgr);
    endwin();

    return 0;
}
