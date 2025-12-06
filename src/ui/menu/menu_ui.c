#include "menu_ui.h"
#include "../core/theme.h"
#include <string.h>

void menu_ui_init(MenuUI *menu)
{
    if (!menu)
        return;
    menu->selected = MENU_SINGLEPLAY;
    menu->option_count = 6;
    menu->current_page = 0;
    menu->total_pages = (menu->option_count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE; // 올림 계산
}

void menu_ui_draw_logo(WINDOW *win)
{
    int start_y = 3;
    int start_x = 14;

    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

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

    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
}

void menu_ui_draw_options(WINDOW *win, const MenuUI *menu)
{
    int box_y = 16;
    int box_x = 32;
    int box_width = 33;
    int box_height = 7; // 3개 아이템 + 여백

    // Draw box - 테마 주색상
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
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
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 메뉴 이름 배열
    const char *menu_names[] = {
        "1 Player Game (COM)",
        "2 Player Game (LAN)",
        "Spectate Game (LAN)",
        "View Replay",
        "Theme",
        "Exit"};

    // 현재 페이지에 표시할 아이템 계산
    int start_idx = menu->current_page * ITEMS_PER_PAGE;
    int end_idx = start_idx + ITEMS_PER_PAGE;
    if (end_idx > menu->option_count)
        end_idx = menu->option_count;

    // Draw options (현재 페이지의 아이템만) - 테마 주색상
    int option_y = box_y + 2;
    int option_x = box_x + 3;

    for (int i = start_idx; i < end_idx; i++)
    {
        int display_row = option_y + (i - start_idx);
        wmove(win, display_row, option_x);

        if (menu->selected == i)
        {
            wattron(win, A_BOLD | A_REVERSE | COLOR_PAIR(COLOR_PAIR_DEFAULT));
            wprintw(win, " ▶  %-25s", menu_names[i]);
            wattroff(win, A_BOLD | A_REVERSE | COLOR_PAIR(COLOR_PAIR_DEFAULT));
        }
        else
        {
            wattron(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
            wprintw(win, "    %-25s", menu_names[i]);
            wattroff(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
        }
    }

    // 페이지 표시 - 테마 INFO 색상
    wattron(win, COLOR_PAIR(COLOR_PAIR_INFO));
    mvwprintw(win, box_y + box_height - 1, box_x + box_width / 2 - 4, " Page %d/%d ",
              menu->current_page + 1, menu->total_pages);
    wattroff(win, COLOR_PAIR(COLOR_PAIR_INFO));

    // Instructions - 테마 DIM 색상
    wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
    mvwprintw(win, box_y + box_height + 1, box_x - 1, "↑↓:Move  ←→:Page  ↵:Select  Q:Exit");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
}

void menu_ui_draw_footer(WINDOW *win)
{
    // int max_y, max_x;
    // getmaxyx(win, max_y, max_x);
    int max_y = 31;
    int max_x = 100;

    wattron(win, COLOR_PAIR(COLOR_PAIR_INFO));
    mvwprintw(win, max_y - 3, 29, "https://github.com/whitedev7773/gomoku-c");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_INFO));
}

void menu_ui_render(WINDOW *win, const MenuUI *menu)
{
    if (!win || !menu)
        return;

    wclear(win);

    // Ensure layout fits 100x31 and draw a centered border of that size
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    if (max_x < 100 || max_y < 31)
    {
        wattron(win, A_BOLD);
        mvwprintw(win, max_y / 2 - 1, (max_x - 18) / 2, "Terminal too small!");
        mvwprintw(win, max_y / 2, (max_x - 36) / 2, "Requires at least 100x31 (current %dx%d)", max_x, max_y);
        mvwprintw(win, max_y / 2 + 1, (max_x - 26) / 2, "Resize terminal and try again.");
        wattroff(win, A_BOLD);
        return;
    }

    int box_w = 100;
    int box_h = 31;
    int start_y = 0;
    int start_x = 0;

    // Border - 테마 주색상
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
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
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    menu_ui_draw_logo(win);
    menu_ui_draw_options(win, menu);
    menu_ui_draw_footer(win);

    wrefresh(win);
}

// 옵션 영역만 재렌더링 (로고, footer 유지)
void menu_ui_render_options_only(WINDOW *win, const MenuUI *menu)
{
    if (!win || !menu)
        return;

    menu_ui_draw_options(win, menu);
    wrefresh(win);
}

void menu_ui_move_selection(MenuUI *menu, int direction)
{
    if (!menu)
        return;

    // 현재 페이지의 범위 계산
    int page_start = menu->current_page * ITEMS_PER_PAGE;
    int page_end = page_start + ITEMS_PER_PAGE - 1;
    if (page_end >= menu->option_count)
        page_end = menu->option_count - 1;

    menu->selected += direction;

    // 현재 페이지 범위 내에서만 이동
    if (menu->selected < page_start)
    {
        menu->selected = page_start;
    }
    else if (menu->selected > page_end)
    {
        menu->selected = page_end;
    }
}

void menu_ui_change_page(MenuUI *menu, int direction)
{
    if (!menu)
        return;

    menu->current_page += direction;

    // 페이지 순환
    if (menu->current_page < 0)
    {
        menu->current_page = menu->total_pages - 1;
    }
    else if (menu->current_page >= menu->total_pages)
    {
        menu->current_page = 0;
    }

    // 페이지 변경 시 해당 페이지의 첫 번째 아이템 선택
    menu->selected = menu->current_page * ITEMS_PER_PAGE;
}

MenuOption menu_ui_get_selected(const MenuUI *menu)
{
    if (!menu)
        return MENU_SINGLEPLAY;
    return menu->selected;
}
