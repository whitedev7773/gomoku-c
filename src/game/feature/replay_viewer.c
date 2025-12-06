#include "replay_viewer.h"
#include "../../ui/core/ui_manager.h"
#include "../../ui/game/board/board_ui.h"
#include "../../ui/game/game_info_ui.h"
#include "../../ui/core/theme.h"
#include "../../ui/core/input_handler.h"
#include "../core/game_logic.h"
#include "../core/turn_manager.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ncurses.h>

// 리플레이 로그 관련 상수
#define MAX_REPLAY_LOG_LINES 100
#define REPLAY_LOG_LINE_LEN 50

// 리플레이 로그 구조체
typedef struct
{
    char lines[MAX_REPLAY_LOG_LINES][REPLAY_LOG_LINE_LEN];
    int count;      // 총 로그 개수
    int scroll_pos; // 스크롤 위치 (표시 시작 인덱스)
} ReplayLog;

// 리플레이 로그 초기화
static void replay_log_init(ReplayLog *log)
{
    memset(log, 0, sizeof(ReplayLog));
    log->count = 0;
    log->scroll_pos = 0;
}

// 리플레이 로그에 항목 추가
static void replay_log_add(ReplayLog *log, const char *message)
{
    if (log->count < MAX_REPLAY_LOG_LINES)
    {
        strncpy(log->lines[log->count], message, REPLAY_LOG_LINE_LEN - 1);
        log->lines[log->count][REPLAY_LOG_LINE_LEN - 1] = '\0';
        log->count++;
    }
    else
    {
        // 꽉 찼으면 위로 밀고 마지막에 추가
        for (int i = 0; i < MAX_REPLAY_LOG_LINES - 1; i++)
        {
            strcpy(log->lines[i], log->lines[i + 1]);
        }
        strncpy(log->lines[MAX_REPLAY_LOG_LINES - 1], message, REPLAY_LOG_LINE_LEN - 1);
        log->lines[MAX_REPLAY_LOG_LINES - 1][REPLAY_LOG_LINE_LEN - 1] = '\0';
    }
}

// 리플레이 로그를 특정 수까지 동기화
static void replay_log_sync(ReplayLog *log, const ReplayState *replay, int target_move)
{
    replay_log_init(log);
    for (int i = 0; i < target_move && i < replay->total_moves; i++)
    {
        const LogEntry *entry = &replay->logger.entries[i];
        const char *player_str = (entry->player == BLACK) ? "Black" : "White";
        char col_char = 'A' + entry->col;
        char msg[REPLAY_LOG_LINE_LEN];
        snprintf(msg, sizeof(msg), "%d. %s placed at %c%d", i + 1, player_str, col_char, entry->row + 1);
        replay_log_add(log, msg);
    }
}

// 리플레이 로그 렌더링
static void replay_log_render(WINDOW *win, const ReplayLog *log)
{
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    // 표시 가능한 최대 라인 수 (테두리 제외)
    int visible_lines = max_y - 2;

    // 스크롤 위치 계산 (항상 최신 로그가 보이도록)
    int start_idx = 0;
    if (log->count > visible_lines)
    {
        start_idx = log->count - visible_lines;
    }

    // 로그 창 내용 지우기
    for (int i = 1; i < max_y - 1; i++)
    {
        mvwhline(win, i, 1, ' ', max_x - 2);
    }

    // 로그 출력
    for (int i = 0; i < visible_lines && (start_idx + i) < log->count; i++)
    {
        const char *line = log->lines[start_idx + i];
        // Black/White에 따라 색상 적용
        if (strstr(line, "Black") != NULL)
        {
            wattron(win, COLOR_PAIR(COLOR_PAIR_BLACK_STONE) | A_BOLD);
        }
        else if (strstr(line, "White") != NULL)
        {
            wattron(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_BOLD);
        }
        else
        {
            wattron(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
        }
        mvwprintw(win, 1 + i, 2, "%-*.*s", max_x - 4, max_x - 4, line);
        wattroff(win, COLOR_PAIR(COLOR_PAIR_BLACK_STONE) | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_BOLD);
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    }

    wrefresh(win);
}

// 리플레이 초기화
bool replay_init(ReplayState *replay, const char *log_filename)
{
    if (!replay || !log_filename)
        return false;

    memset(replay, 0, sizeof(ReplayState));

    if (!logger_load_from_file(&replay->logger, log_filename))
    {
        return false;
    }

    replay->current_move = 0;
    replay->total_moves = replay->logger.entry_count;
    replay->playing = false;
    replay->paused = false;
    replay->speed = REPLAY_SPEED_NORMAL;

    return true;
}

// 리플레이 정리
void replay_cleanup(ReplayState *replay)
{
    if (!replay)
        return;
    // 추가 정리 작업이 필요하면 여기에
}

// 다음 수 재생
bool replay_next_move(ReplayState *replay, Board *board)
{
    if (!replay || !board)
        return false;

    if (replay->current_move >= replay->total_moves)
    {
        return false; // 더 이상 수가 없음
    }

    const LogEntry *entry = &replay->logger.entries[replay->current_move];

    if (board_place_stone(board, entry->row, entry->col, entry->player))
    {
        replay->current_move++;
        return true;
    }

    return false;
}

// 이전 수로 되돌리기
bool replay_prev_move(ReplayState *replay, Board *board)
{
    if (!replay || !board)
        return false;

    if (replay->current_move <= 0)
    {
        return false; // 더 이상 되돌릴 수 없음
    }

    // 보드를 초기화하고 현재 수 -1까지 다시 재생
    board_clear(board);

    int target_move = replay->current_move - 1;
    replay->current_move = 0;

    for (int i = 0; i < target_move; i++)
    {
        const LogEntry *entry = &replay->logger.entries[i];
        board_place_stone(board, entry->row, entry->col, entry->player);
        replay->current_move++;
    }

    return true;
}

// 리플레이 UI 렌더링
static void replay_render(UIManager *ui_mgr, const Board *board, const ReplayState *replay, const ReplayLog *log)
{
    // 보드 렌더링
    board_render_full(ui_mgr->board_win, board, NULL);

    // 정보 렌더링
    werase(ui_mgr->info_win);
    box(ui_mgr->info_win, 0, 0);
    mvwprintw(ui_mgr->info_win, 0, 2, " Replay ");

    mvwprintw(ui_mgr->info_win, 2, 2, "File: %s", replay->logger.filename);
    mvwprintw(ui_mgr->info_win, 4, 2, "Move: %d / %d",
              replay->current_move, replay->total_moves);

    // 진행률 바
    int progress_width = 30;
    int filled = (replay->total_moves > 0) ? (replay->current_move * progress_width / replay->total_moves) : 0;

    mvwprintw(ui_mgr->info_win, 6, 2, "Progress: [");
    for (int i = 0; i < progress_width; i++)
    {
        if (i < filled)
        {
            waddch(ui_mgr->info_win, '=');
        }
        else
        {
            waddch(ui_mgr->info_win, '-');
        }
    }
    wprintw(ui_mgr->info_win, "]");

    // 현재 수 정보
    if (replay->current_move > 0)
    {
        const LogEntry *entry = &replay->logger.entries[replay->current_move - 1];
        const char *player_str = (entry->player == BLACK) ? "BLACK" : "WHITE";
        char col_char = 'A' + entry->col;

        mvwprintw(ui_mgr->info_win, 8, 2, "Last Move: %s at %c%02d",
                  player_str, col_char, entry->row);
    }

    // 재생 상태
    const char *status = replay->playing ? (replay->paused ? "PAUSED" : "PLAYING") : "STOPPED";
    mvwprintw(ui_mgr->info_win, 10, 2, "Status: %s", status);

    // 속도
    const char *speed_str;
    if (replay->speed == REPLAY_SPEED_SLOW)
        speed_str = "SLOW";
    else if (replay->speed == REPLAY_SPEED_FAST)
        speed_str = "FAST";
    else
        speed_str = "NORMAL";
    mvwprintw(ui_mgr->info_win, 11, 2, "Speed: %s", speed_str);

    // 조작 안내
    int max_y;
    getmaxyx(ui_mgr->info_win, max_y, max_y);
    mvwprintw(ui_mgr->info_win, max_y - 7, 2, "Controls:");
    mvwprintw(ui_mgr->info_win, max_y - 6, 2, "  SPACE - Play/Pause");
    mvwprintw(ui_mgr->info_win, max_y - 5, 2, "  -> - Next move");
    mvwprintw(ui_mgr->info_win, max_y - 4, 2, "  <- - Prev move");
    mvwprintw(ui_mgr->info_win, max_y - 3, 2, "  1/2/3 - Speed");
    mvwprintw(ui_mgr->info_win, max_y - 2, 2, "  q - Quit");

    wrefresh(ui_mgr->info_win);
    wrefresh(ui_mgr->board_win);

    // 로그 창 렌더링 (chat_win 사용)
    wattron(ui_mgr->chat_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    box(ui_mgr->chat_win, 0, 0);
    mvwprintw(ui_mgr->chat_win, 0, 2, " Move Log ");
    wattroff(ui_mgr->chat_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    replay_log_render(ui_mgr->chat_win, log);
}

// 리플레이 실행
int replay_run(const char *log_filename)
{
    ReplayState replay;
    if (!replay_init(&replay, log_filename))
    {
        printf("Failed to load replay file: %s\n", log_filename);
        return 1;
    }

    // UI 초기화
    UIManager ui_mgr;
    ui_manager_init(&ui_mgr);

    // 게임패드 입력 핸들러 초기화
    InputHandler input_handler;
    input_handler_init(&input_handler);

    Board board;
    board_init(&board);

    // 리플레이 로그 초기화
    ReplayLog move_log;
    replay_log_init(&move_log);

    keypad(ui_mgr.board_win, TRUE);
    wtimeout(ui_mgr.board_win, 50); // 50ms timeout for non-blocking input

    bool running = true;
    bool needs_render = true; // 렌더링 필요 여부 플래그

    // 초기 렌더링
    replay_render(&ui_mgr, &board, &replay, &move_log);

    while (running)
    {
        // 자동 재생
        if (replay.playing && !replay.paused)
        {
            struct timespec ts;
            ts.tv_sec = replay.speed / 1000000;
            ts.tv_nsec = (replay.speed % 1000000) * 1000;
            nanosleep(&ts, NULL);

            if (replay_next_move(&replay, &board))
            {
                // 로그에 추가
                const LogEntry *entry = &replay.logger.entries[replay.current_move - 1];
                const char *player_str = (entry->player == BLACK) ? "Black" : "White";
                char col_char = 'A' + entry->col;
                char msg[REPLAY_LOG_LINE_LEN];
                snprintf(msg, sizeof(msg), "%d. %s placed at %c%d", replay.current_move, player_str, col_char, entry->row + 1);
                replay_log_add(&move_log, msg);
                needs_render = true;
            }
            else
            {
                // 끝까지 재생 완료
                replay.playing = false;
                needs_render = true;
            }
        }

        // 변경이 있을 때만 렌더링
        if (needs_render)
        {
            replay_render(&ui_mgr, &board, &replay, &move_log);
            needs_render = false;
        }

        // 입력 처리
        InputEvent event = input_handler_get_event(&input_handler, ui_mgr.board_win);
        int ch = event.key_code;
        InputAction action = event.action;
        switch (action)
        {
        case INPUT_PLACE_STONE: // Space - Play/Pause (A button on gamepad)
            if (replay.playing)
            {
                replay.paused = !replay.paused;
            }
            else
            {
                replay.playing = true;
                replay.paused = false;
            }
            needs_render = true;
            break;

        case INPUT_MOVE_RIGHT: // 다음 수
            replay.playing = false;
            if (replay_next_move(&replay, &board))
            {
                // 로그에 추가
                const LogEntry *entry = &replay.logger.entries[replay.current_move - 1];
                const char *player_str = (entry->player == BLACK) ? "Black" : "White";
                char col_char = 'A' + entry->col;
                char msg[REPLAY_LOG_LINE_LEN];
                snprintf(msg, sizeof(msg), "%d. %s placed at %c%d", replay.current_move, player_str, col_char, entry->row + 1);
                replay_log_add(&move_log, msg);
                needs_render = true;
            }
            break;

        case INPUT_MOVE_LEFT: // 이전 수
            replay.playing = false;
            if (replay_prev_move(&replay, &board))
            {
                // 로그를 현재 수까지만 동기화
                replay_log_sync(&move_log, &replay, replay.current_move);
                needs_render = true;
            }
            break;

        case INPUT_QUIT: // Quit
            running = false;
            break;

        default:
            break;
        }

        // Legacy key handling (for speed controls)
        switch (ch)
        {
        case '1': // Slow speed
            replay.speed = REPLAY_SPEED_SLOW;
            needs_render = true;
            break;

        case '2': // Normal speed
            replay.speed = REPLAY_SPEED_NORMAL;
            needs_render = true;
            break;

        case '3': // Fast speed
            replay.speed = REPLAY_SPEED_FAST;
            needs_render = true;
            break;
        }
    }

    // 정리
    input_handler_cleanup(&input_handler);
    replay_cleanup(&replay);
    ui_manager_cleanup(&ui_mgr);

    return 0;
}
