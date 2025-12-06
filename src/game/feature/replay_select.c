#include "replay_select.h"
#include "replay_viewer.h"
#include "../../ui/menu/replay_list_ui.h"
#include "../../ui/core/theme.h"
#include "../../ui/core/input_handler.h"
#include <stdio.h>
#include <string.h>
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
        // 창 크기 가져오기
        int max_y, max_x;
        getmaxyx(list_win, max_y, max_x);

        werase(list_win);

        // 테마 주색상으로 border
        wattron(list_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
        box(list_win, 0, 0);
        wattroff(list_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));

        // 테마 주색상으로 타이틀
        wattron(list_win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
        mvwprintw(list_win, 0, 2, " Select Replay File ");
        wattroff(list_win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

        // 빈 폴더 아이콘 - 테마 주색상 적용
        int center_y = max_y / 2 - 6;
        int center_x = (max_x - 16) / 2;

        wattron(list_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
        mvwprintw(list_win, center_y + 0, center_x, " ╭────────╮     ");
        mvwprintw(list_win, center_y + 1, center_x, "╭╯        ╰─────╮");
        mvwprintw(list_win, center_y + 2, center_x, "│               │");
        mvwprintw(list_win, center_y + 3, center_x, "│    O     O    │");
        mvwprintw(list_win, center_y + 4, center_x, "│       ^       │");
        mvwprintw(list_win, center_y + 5, center_x, "╰───────────────╯");
        wattroff(list_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));

        // 메시지 - 테마 INFO 색상
        wattron(list_win, A_BOLD | COLOR_PAIR(COLOR_PAIR_INFO));
        mvwprintw(list_win, center_y + 7, (max_x - 21) / 2, "No Replay Files Found");
        wattroff(list_win, A_BOLD | COLOR_PAIR(COLOR_PAIR_INFO));

        wattron(list_win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(list_win, center_y + 9, (max_x - 36) / 2, "Play a game first to create replays!");
        wattroff(list_win, COLOR_PAIR(COLOR_PAIR_DIM));

        // 하단 안내
        wattron(list_win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(list_win, max_y - 3, (max_x - 21) / 2, "Press any key to exit");
        wattroff(list_win, COLOR_PAIR(COLOR_PAIR_DIM));

        wrefresh(list_win);
        InputEvent event = input_handler_get_event(&input_handler, list_win);
        input_handler_cleanup(&input_handler);
        delwin(list_win);
        endwin();
        return 1;
    }

    bool running = true;
    const char *selected_file = NULL;
    bool first_render = true;
    int prev_selected = list_ui.selected_index;

    while (running)
    {
        // 파일 선택 루프
        bool selecting = true;
        selected_file = NULL;

        while (selecting)
        {
            if (first_render)
            {
                replay_list_ui_render(list_win, &list_ui);
                first_render = false;
            }
            else
            {
                replay_list_ui_render_selection_only(list_win, &list_ui, prev_selected);
            }
            prev_selected = list_ui.selected_index;

            InputEvent event = input_handler_get_event(&input_handler, list_win);
            switch (event.action)
            {
            case INPUT_MOVE_UP:
                if (replay_list_ui_move_selection(&list_ui, -1))
                {
                    first_render = true; // 스크롤 발생 시 전체 재렌더링
                }
                break;

            case INPUT_MOVE_DOWN:
                if (replay_list_ui_move_selection(&list_ui, 1))
                {
                    first_render = true; // 스크롤 발생 시 전체 재렌더링
                }
                break;

            case INPUT_PLACE_STONE:
                selected_file = replay_list_ui_get_selected_file(&list_ui);
                selecting = false;
                break;

            case INPUT_QUIT:
                selecting = false;
                running = false;
                break;

            default:
                break;
            }
        }

        // 파일이 선택되었으면 리플레이 실행
        if (selected_file && running)
        {
            // ncurses 종료
            delwin(list_win);
            endwin();

            // 리플레이 실행
            replay_run(selected_file);

            // 다시 ncurses 초기화 (파일 선택 화면으로 돌아감)
            initscr();
            cbreak();
            noecho();
            curs_set(0);
            theme_init(theme_get_current());

            list_win = newwin(30, 100, 0, 0);
            keypad(list_win, TRUE);

            first_render = true; // 전체 재렌더링 필요
        }
    }

    input_handler_cleanup(&input_handler);
    delwin(list_win);
    endwin();

    return 0;
}
