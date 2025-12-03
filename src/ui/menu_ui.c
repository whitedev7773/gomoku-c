#include "menu_ui.h"
#include <string.h>

void menu_ui_init(MenuUI *menu)
{
    if (!menu)
        return;
    menu->selected = MENU_SINGLEPLAY;
    menu->option_count = 5;
}

void menu_ui_draw_logo(WINDOW *win)
{
    int start_y = 3;
    int start_x = 14;

    wattron(win, A_BOLD);

    // GO
    mvwprintw(win, start_y + 0, start_x, "     ████████████         ████████████████████             █████████████");
    mvwprintw(win, start_y + 1, start_x, "   ████\\______/████       ████\\__________/████           ████__________/");
    mvwprintw(win, start_y + 2, start_x, " ████\\_|      |_/████     ████|          |████         ████_/           ");
    mvwprintw(win, start_y + 3, start_x, " ████|          |████     ████████████████████         ██_/             ");
    mvwprintw(win, start_y + 4, start_x, " ████|          |████     \\________██________/         ██|              ");
    mvwprintw(win, start_y + 5, start_x, " \\_████        ████_/   ████████████████████████       ██|              ");
    mvwprintw(win, start_y + 6, start_x, "   \\_████████████_/     \\______________________/       ██|              ");
    mvwprintw(win, start_y + 7, start_x, "     \\____██____/         ████████████████████         ████             ");
    mvwprintw(win, start_y + 8, start_x, "          ██              \\_______________████         \\_████           ");
    mvwprintw(win, start_y + 9, start_x, "██████████████████████                    ████    ███    \\_█████████████");
    mvwprintw(win, start_y + 10, start_x, "\\____________________/                    \\__/    \\_/      \\___________/");

    wattroff(win, A_BOLD);
}

void menu_ui_draw_options(WINDOW *win, const MenuUI *menu)
{
    int box_y = 16;
    int box_x = 32;
    int box_width = 33;
    int box_height = 8;

    // Draw box
    wattron(win, A_BOLD);
    mvwprintw(win, box_y, box_x, "┏");
    for (int i = 1; i < box_width - 1; i++)
    {
        wprintw(win, "━");
    }
    wprintw(win, "┓");

    for (int i = 1; i < box_height - 1; i++)
    {
        mvwprintw(win, box_y + i, box_x, "┃");
        mvwprintw(win, box_y + i, box_x + box_width - 1, "┃");
    }

    mvwprintw(win, box_y + box_height - 1, box_x, "┗");
    for (int i = 1; i < box_width - 1; i++)
    {
        wprintw(win, "━");
    }
    wprintw(win, "┛");
    wattroff(win, A_BOLD);

    // Draw options
    int option_y = box_y + 2;
    int option_x = box_x + 3;

    // Option 1: 1 Player Game (COM)
    wmove(win, option_y, option_x);
    if (menu->selected == MENU_SINGLEPLAY)
    {
        wattron(win, A_BOLD | COLOR_PAIR(4));
        wprintw(win, " ▶  1 Player Game (COM)      ");
        wattroff(win, A_BOLD | COLOR_PAIR(4));
    }
    else
    {
        wprintw(win, "    1 Player Game (COM)      ");
    }

    // Option 2: 2 Player Game (LAN)
    wmove(win, option_y + 1, option_x);
    if (menu->selected == MENU_MULTIPLAY)
    {
        wattron(win, A_BOLD | COLOR_PAIR(4));
        wprintw(win, " ▶  2 Player Game (LAN)      ");
        wattroff(win, A_BOLD | COLOR_PAIR(4));
    }
    else
    {
        wprintw(win, "    2 Player Game (LAN)      ");
    }

    // Option 3: Spectate Game (LAN)
    wmove(win, option_y + 2, option_x);
    if (menu->selected == MENU_SPECTATOR)
    {
        wattron(win, A_BOLD | COLOR_PAIR(4));
        wprintw(win, " ▶  Spectate Game (LAN)      ");
        wattroff(win, A_BOLD | COLOR_PAIR(4));
    }
    else
    {
        wprintw(win, "    Spectate Game (LAN)      ");
    }

    // Option 4: View Replay
    wmove(win, option_y + 3, option_x);
    if (menu->selected == MENU_REPLAY)
    {
        wattron(win, A_BOLD | COLOR_PAIR(4));
        wprintw(win, " ▶  View Replay              ");
        wattroff(win, A_BOLD | COLOR_PAIR(4));
    }
    else
    {
        wprintw(win, "    View Replay              ");
    }

    // Instructions
    wattron(win, COLOR_PAIR(8)); // gray
    mvwprintw(win, box_y + box_height + 1, box_x - 8, "↑↓:Move    ↵:Select    Q:Exit");
    wattroff(win, COLOR_PAIR(8));
}

void menu_ui_draw_footer(WINDOW *win)
{
    // int max_y, max_x;
    // getmaxyx(win, max_y, max_x);
    int max_y = 30;
    int max_x = 100;

    wattron(win, COLOR_PAIR(7));
    mvwprintw(win, max_y - 3, 29, "https://github.com/whitedev7773/gomoku-c");
    wattroff(win, COLOR_PAIR(7));
}

void menu_ui_render(WINDOW *win, const MenuUI *menu)
{
    if (!win || !menu)
        return;

    wclear(win);

    // Ensure layout fits 100x30 and draw a centered border of that size
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    if (max_x < 100 || max_y < 30)
    {
        wattron(win, A_BOLD);
        mvwprintw(win, max_y / 2 - 1, (max_x - 18) / 2, "Terminal too small!");
        mvwprintw(win, max_y / 2, (max_x - 36) / 2, "Requires at least 100x30 (current %dx%d)", max_x, max_y);
        mvwprintw(win, max_y / 2 + 1, (max_x - 26) / 2, "Resize terminal and try again.");
        wattroff(win, A_BOLD);
        return;
    }

    int box_w = 100;
    int box_h = 30;
    int start_y = 0;
    int start_x = 0;

    wattron(win, A_BOLD);
    // Top border
    mvwaddch(win, start_y, start_x, ACS_ULCORNER);
    for (int i = 1; i < box_w - 1; ++i)
        mvwaddch(win, start_y, start_x + i, ACS_HLINE);
    mvwaddch(win, start_y, start_x + box_w - 1, ACS_URCORNER);

    // Side borders
    for (int j = 1; j < box_h - 1; ++j)
    {
        mvwaddch(win, start_y + j, start_x, ACS_VLINE);
        mvwaddch(win, start_y + j, start_x + box_w - 1, ACS_VLINE);
    }

    // Bottom border
    mvwaddch(win, start_y + box_h - 1, start_x, ACS_LLCORNER);
    for (int i = 1; i < box_w - 1; ++i)
        mvwaddch(win, start_y + box_h - 1, start_x + i, ACS_HLINE);
    mvwaddch(win, start_y + box_h - 1, start_x + box_w - 1, ACS_LRCORNER);
    wattroff(win, A_BOLD);

    menu_ui_draw_logo(win);
    menu_ui_draw_options(win, menu);
    menu_ui_draw_footer(win);

    wrefresh(win);
}

void menu_ui_move_selection(MenuUI *menu, int direction)
{
    if (!menu)
        return;

    menu->selected += direction;

    if (menu->selected < 0)
    {
        menu->selected = MENU_REPLAY;
    }
    else if (menu->selected > menu->option_count - 2)
    {
        menu->selected = MENU_SINGLEPLAY;
    }
}

MenuOption menu_ui_get_selected(const MenuUI *menu)
{
    if (!menu)
        return MENU_SINGLEPLAY;
    return menu->selected;
}
