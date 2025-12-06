#include <stdio.h>
#include <locale.h>
#include "../ui/menu/menu_ui.h"

int main()
{
    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    curs_set(0);

    // Limit stdscr to 100x31
    wresize(stdscr, 31, 100);

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

    // Always use fixed 100x31 size
    WINDOW *menu_win = newwin(31, 100, 0, 0);
    keypad(menu_win, TRUE);

    MenuUI menu;
    menu_ui_init(&menu);

    bool running = true;

    while (running)
    {
        menu_ui_render(menu_win, &menu);

        int ch = wgetch(menu_win);

        switch (ch)
        {
        case KEY_UP:
            menu_ui_move_selection(&menu, -1);
            break;
        case KEY_DOWN:
            menu_ui_move_selection(&menu, 1);
            break;
        case '\n':
        case KEY_ENTER:
            running = false;
            break;
        case 'q':
        case 'Q':
            running = false;
            break;
        }
    }

    delwin(menu_win);
    endwin();

    MenuOption selected = menu_ui_get_selected(&menu);
    printf("Selected: %s\n",
           selected == MENU_SINGLEPLAY ? "1 Player Game" : "2 Player Game");

    return 0;
}
