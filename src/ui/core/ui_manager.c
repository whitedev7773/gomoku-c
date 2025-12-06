#include "ui_manager.h"
#include "theme.h"
#include <stdlib.h>
#include <locale.h>
#include <string.h>

// ============================================
// Dirty Flag 관리 함수 구현
// ============================================

void ui_render_flags_init(UIRenderFlags *flags)
{
    if (!flags)
        return;
    flags->flags = RENDER_ALL; // 초기에는 모든 것을 렌더링
    flags->dirty_cell_count = 0;
    memset(flags->dirty_cells, 0, sizeof(flags->dirty_cells));
}

void ui_render_flags_set(UIRenderFlags *flags, unsigned int flag)
{
    if (!flags)
        return;
    flags->flags |= flag;
}

void ui_render_flags_clear(UIRenderFlags *flags, unsigned int flag)
{
    if (!flags)
        return;
    flags->flags &= ~flag;
}

void ui_render_flags_clear_all(UIRenderFlags *flags)
{
    if (!flags)
        return;
    flags->flags = RENDER_NONE;
    flags->dirty_cell_count = 0;
}

bool ui_render_flags_is_set(const UIRenderFlags *flags, unsigned int flag)
{
    if (!flags)
        return false;
    return (flags->flags & flag) != 0;
}

void ui_render_flags_add_dirty_cell(UIRenderFlags *flags, int row, int col)
{
    if (!flags)
        return;
    if (flags->dirty_cell_count >= 361)
        return; // 최대 19x19

    // 중복 체크
    for (int i = 0; i < flags->dirty_cell_count; i++)
    {
        if (flags->dirty_cells[i][0] == row && flags->dirty_cells[i][1] == col)
        {
            return; // 이미 존재
        }
    }

    flags->dirty_cells[flags->dirty_cell_count][0] = row;
    flags->dirty_cells[flags->dirty_cell_count][1] = col;
    flags->dirty_cell_count++;
    flags->flags |= RENDER_BOARD_CELL;
}

void ui_render_flags_clear_dirty_cells(UIRenderFlags *flags)
{
    if (!flags)
        return;
    flags->dirty_cell_count = 0;
}

void ui_render_flags_set_all(UIRenderFlags *flags)
{
    if (!flags)
        return;
    flags->flags = RENDER_ALL;
}

bool ui_manager_check_terminal_size(void)
{
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    return (max_x >= UI_MIN_WIDTH && max_y >= UI_MIN_HEIGHT);
}

bool ui_manager_init(UIManager *manager)
{
    if (!manager)
        return false;

    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    curs_set(0);

    // Limit stdscr to 100x31
    wresize(stdscr, UI_MIN_HEIGHT, UI_MIN_WIDTH);
    keypad(stdscr, TRUE);

    if (!ui_manager_check_terminal_size())
    {
        endwin();
        return false;
    }

    // 테마 초기화
    theme_init(theme_get_current());

    manager->board_win = newwin(BOARD_WINDOW_HEIGHT, BOARD_WINDOW_WIDTH,
                                BOARD_WINDOW_Y, BOARD_WINDOW_X);
    manager->info_win = newwin(INFO_WINDOW_HEIGHT, INFO_WINDOW_WIDTH,
                               INFO_WINDOW_Y, INFO_WINDOW_X);
    manager->chat_win = newwin(CHAT_WINDOW_HEIGHT, CHAT_WINDOW_WIDTH,
                               CHAT_WINDOW_Y, CHAT_WINDOW_X);
    manager->chat_input_win = newwin(CHAT_INPUT_HEIGHT, CHAT_INPUT_WIDTH,
                                     CHAT_INPUT_Y, CHAT_INPUT_X);
    manager->bottom_win = newwin(BOTTOM_WINDOW_HEIGHT, BOTTOM_WINDOW_WIDTH,
                                 BOTTOM_WINDOW_Y, BOTTOM_WINDOW_X);

    if (!manager->board_win || !manager->info_win || !manager->chat_win ||
        !manager->chat_input_win || !manager->bottom_win)
    {
        ui_manager_cleanup(manager);
        return false;
    }

    keypad(manager->board_win, TRUE);
    nodelay(manager->board_win, FALSE);

    manager->initialized = true;
    manager->first_render = true; // 첫 렌더링 표시

    // 렌더링 플래그 초기화 (모든 것을 렌더링)
    ui_render_flags_init(&manager->render_flags);

    return true;
}

void ui_manager_cleanup(UIManager *manager)
{
    if (!manager)
        return;

    if (manager->board_win)
        delwin(manager->board_win);
    if (manager->info_win)
        delwin(manager->info_win);
    if (manager->chat_win)
        delwin(manager->chat_win);
    if (manager->chat_input_win)
        delwin(manager->chat_input_win);
    if (manager->bottom_win)
    {
    }
    // delwin(manager->bottom_win);

    endwin();
    manager->initialized = false;
}

void ui_manager_refresh_all(UIManager *manager)
{
    if (!manager || !manager->initialized)
        return;

    wrefresh(manager->board_win);
    wrefresh(manager->info_win);
    wrefresh(manager->chat_win);
    wrefresh(manager->chat_input_win);
    wrefresh(manager->bottom_win);
}

void ui_manager_clear_all(UIManager *manager)
{
    if (!manager || !manager->initialized)
        return;

    wclear(manager->board_win);
    wclear(manager->info_win);
    wclear(manager->chat_win);
    wclear(manager->chat_input_win);
    wclear(manager->bottom_win);
}
