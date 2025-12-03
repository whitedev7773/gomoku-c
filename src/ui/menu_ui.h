#ifndef MENU_UI_H
#define MENU_UI_H

#include <ncurses.h>

typedef enum {
    MENU_SINGLEPLAY = 0,
    MENU_MULTIPLAY = 1,
    MENU_SPECTATOR = 2,
    MENU_REPLAY = 3,
    MENU_EXIT = 4
} MenuOption;

typedef struct {
    MenuOption selected;
    int option_count;
} MenuUI;

void menu_ui_init(MenuUI *menu);

void menu_ui_render(WINDOW *win, const MenuUI *menu);

void menu_ui_draw_logo(WINDOW *win);

void menu_ui_draw_options(WINDOW *win, const MenuUI *menu);

void menu_ui_draw_footer(WINDOW *win);

void menu_ui_move_selection(MenuUI *menu, int direction);

MenuOption menu_ui_get_selected(const MenuUI *menu);

#endif // MENU_UI_H
