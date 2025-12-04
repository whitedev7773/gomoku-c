#include "replay.h"
#include "../../ui/core/ui_manager.h"
#include "../../ui/game/board_ui.h"
#include "../../ui/game/game_info_ui.h"
#include "../../ui/menu/replay_list_ui.h"
#include "../../ui/core/theme.h"
#include "../../ui/core/input_handler.h"
#include "../core/game_logic.h"
#include "../core/turn_manager.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <locale.h>
#include <ncurses.h>

// 로그 파일 목록 가져오기
bool replay_get_log_files(LogFileList *list)
{
    if (!list)
        return false;

    memset(list, 0, sizeof(LogFileList));

    DIR *dir = opendir(".");
    if (!dir)
        return false;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && list->file_count < MAX_LOG_FILES)
    {
        // "gomoku-" 로 시작하고 ".log"로 끝나는 파일 찾기
        if (strncmp(entry->d_name, "gomoku-", 7) == 0 &&
            strstr(entry->d_name, ".log") != NULL)
        {

            LogFileInfo *info = &list->files[list->file_count];
            strncpy(info->filename, entry->d_name, LOG_FILENAME_SIZE - 1);

            // 파일 수정 시간 가져오기
            struct stat file_stat;
            if (stat(entry->d_name, &file_stat) == 0)
            {
                info->modified_time = file_stat.st_mtime;

                // 표시용 이름 생성
                struct tm *t = localtime(&info->modified_time);
                snprintf(info->display_name, sizeof(info->display_name),
                         "%s (%04d-%02d-%02d %02d:%02d)",
                         entry->d_name,
                         t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                         t->tm_hour, t->tm_min);
            }
            else
            {
                strcpy(info->display_name, entry->d_name);
            }

            list->file_count++;
        }
    }

    closedir(dir);

    // 최신 파일이 위로 오도록 정렬 (bubble sort)
    for (int i = 0; i < list->file_count - 1; i++)
    {
        for (int j = 0; j < list->file_count - i - 1; j++)
        {
            if (list->files[j].modified_time < list->files[j + 1].modified_time)
            {
                LogFileInfo temp = list->files[j];
                list->files[j] = list->files[j + 1];
                list->files[j + 1] = temp;
            }
        }
    }

    return list->file_count > 0;
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
static void replay_render(UIManager *ui_mgr, const Board *board, const ReplayState *replay)
{
    // 보드 렌더링
    board_ui_render(ui_mgr->board_win, board, NULL);

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

// 파일 선택 후 리플레이 실행
int replay_run_with_selection(void)
{
    // Locale 설정
    setlocale(LC_ALL, "");

    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    // 테마 초기화
    theme_init(theme_get_current());

    // 파일 선택 UI
    WINDOW *list_win = newwin(30, 100, 0, 0);
    keypad(list_win, TRUE);

    // 게임패드 입력 핸들러 초기화
    InputHandler input_handler;
    input_handler_init(&input_handler);

    ReplayListUI list_ui;
    if (!replay_list_ui_init(&list_ui))
    {
        mvwprintw(list_win, 15, 30, "No replay files found!");
        mvwprintw(list_win, 17, 25, "Press any key to exit...");
        wrefresh(list_win);
        InputEvent event = input_handler_get_event(&input_handler, list_win);
        input_handler_cleanup(&input_handler);
        delwin(list_win);
        endwin();
        return 1;
    }

    bool running = true;
    const char *selected_file = NULL;

    while (running)
    {
        replay_list_ui_render(list_win, &list_ui);

        InputEvent event = input_handler_get_event(&input_handler, list_win);
        switch (event.action)
        {
        case INPUT_MOVE_UP:
            replay_list_ui_move_selection(&list_ui, -1);
            break;

        case INPUT_MOVE_DOWN:
            replay_list_ui_move_selection(&list_ui, 1);
            break;

        case INPUT_PLACE_STONE:
            selected_file = replay_list_ui_get_selected_file(&list_ui);
            running = false;
            break;

        case INPUT_QUIT:
            running = false;
            break;

        default:
            break;
        }
    }

    input_handler_cleanup(&input_handler);
    delwin(list_win);
    endwin();

    // 파일이 선택되었으면 리플레이 실행
    if (selected_file)
    {
        return replay_run(selected_file);
    }

    return 0;
}
