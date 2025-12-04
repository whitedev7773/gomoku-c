#ifndef MENU_UI_H
#define MENU_UI_H

#include <ncurses.h>

#define ITEMS_PER_PAGE 3

typedef enum {
    MENU_SINGLEPLAY = 0,
    MENU_MULTIPLAY = 1,
    MENU_SPECTATOR = 2,
    MENU_REPLAY = 3,
    MENU_THEME = 4,
    MENU_EXIT = 5
} MenuOption;

typedef struct {
    MenuOption selected;
    int option_count;
    int current_page;       // 현재 페이지 (0부터 시작)
    int total_pages;        // 총 페이지 수
} MenuUI;

void menu_ui_init(MenuUI *menu);

void menu_ui_render(WINDOW *win, const MenuUI *menu);

void menu_ui_draw_logo(WINDOW *win);

void menu_ui_draw_options(WINDOW *win, const MenuUI *menu);

void menu_ui_draw_footer(WINDOW *win);

void menu_ui_move_selection(MenuUI *menu, int direction);

void menu_ui_change_page(MenuUI *menu, int direction);

MenuOption menu_ui_get_selected(const MenuUI *menu);

#endif // MENU_UI_H
