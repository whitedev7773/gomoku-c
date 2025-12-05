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
static void replay_render(UIManager *ui_mgr, const Board *board, const ReplayState *replay)
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

    keypad(ui_mgr.board_win, TRUE);
    wtimeout(ui_mgr.board_win, 50); // 50ms timeout for non-blocking input

    bool running = true;
    time_t last_auto_play = time(NULL);

    while (running)
    {
        // UI 렌더링
        replay_render(&ui_mgr, &board, &replay);

        // 자동 재생
        if (replay.playing && !replay.paused)
        {
            struct timespec ts;
            ts.tv_sec = replay.speed / 1000000;
            ts.tv_nsec = (replay.speed % 1000000) * 1000;
            nanosleep(&ts, NULL);

            if (!replay_next_move(&replay, &board))
            {
                // 끝까지 재생 완료
                replay.playing = false;
            }
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
            break;

        case INPUT_MOVE_RIGHT: // 다음 수
            replay.playing = false;
            replay_next_move(&replay, &board);
            break;

        case INPUT_MOVE_LEFT: // 이전 수
            replay.playing = false;
            replay_prev_move(&replay, &board);
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
            break;

        case '2': // Normal speed
            replay.speed = REPLAY_SPEED_NORMAL;
            break;

        case '3': // Fast speed
            replay.speed = REPLAY_SPEED_FAST;
            break;
        }
    }

    // 정리
    input_handler_cleanup(&input_handler);
    replay_cleanup(&replay);
    ui_manager_cleanup(&ui_mgr);

    return 0;
}
