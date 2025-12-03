#include "ui_manager.h"
#include <stdlib.h>
#include <locale.h>

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

    // Limit stdscr to 100x30
    wresize(stdscr, UI_MIN_HEIGHT, UI_MIN_WIDTH);
    keypad(stdscr, TRUE);

    if (!ui_manager_check_terminal_size())
    {
        endwin();
        return false;
    }

    if (has_colors())
    {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_BLACK, COLOR_WHITE);
        init_pair(3, COLOR_RED, COLOR_BLACK);
        init_pair(4, COLOR_GREEN, COLOR_BLACK);
        init_pair(5, COLOR_YELLOW, COLOR_BLACK);
        init_pair(6, COLOR_BLUE, COLOR_BLACK);
        init_pair(7, COLOR_CYAN, COLOR_BLACK);
        init_pair(8, COLOR_WHITE, COLOR_BLACK);
    }

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
        delwin(manager->bottom_win);

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
