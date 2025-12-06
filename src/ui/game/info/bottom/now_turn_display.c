#include "now_turn_display.h"
#include "../../../core/theme.h"
#include <stdio.h>

void now_turn_display_init(NowTurnDisplay *display)
{
    if (!display)
        return;
    display->prev_current_player = EMPTY;
}

void now_turn_display_draw(WINDOW *win, Stone current_player)
{
    if (!win)
        return;

    // NOW TURN 영역: (18,24) 시작, 15x7
    // 제목은 ingame_border에서 그리므로 여기서는 내용만
    // 내부 콘텐츠 영역: (19,27) ~ (32,29)

    attron(COLOR_PAIR(COLOR_PAIR_DEFAULT) | A_BOLD);
    mvprintw(25, 21, "NOW  TURN  ┃");
    mvprintw(26, 18, "╂─────────────╊━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━");
    mvprintw(27, 32, "┃");
    mvprintw(28, 32, "┃");
    mvprintw(29, 32, "┃");
    attroff(COLOR_PAIR(COLOR_PAIR_DEFAULT) | A_BOLD);

    attron(COLOR_PAIR(COLOR_PAIR_DEFAULT) | A_BOLD);
    int x = 21;
    if (current_player == BLACK)
    {
        mvprintw(27, x, "╻    ╻ ┏╸");
        mvprintw(28, x, "┣━━┓ ┣┫  ");
        mvprintw(29, x, "┗━━┛ ╹ ┗╸");
    }
    else
    {
        mvprintw(27, x, "╻┏┓╻ ━┳━ ");
        mvprintw(28, x, "┃┃┃┃  ┃  ");
        mvprintw(29, x, "┗┛┗┛  ╹  ");
    }
    attroff(COLOR_PAIR(COLOR_PAIR_DEFAULT) | A_BOLD);
    refresh();
}

bool now_turn_display_render(WINDOW *win, const TurnManager *turn_mgr,
                             NowTurnDisplay *display,
                             UIRenderFlags *flags,
                             bool force_render)
{
    if (!win || !turn_mgr || !display || !flags)
        return false;

    bool rendered = false;

    if (force_render || ui_render_flags_is_set(flags, RENDER_CURRENT_TURN))
    {
        Stone current_player = turn_manager_get_current_player(turn_mgr);

        // 변경 여부 확인
        if (force_render || display->prev_current_player != current_player)
        {
            now_turn_display_draw(win, current_player);
            display->prev_current_player = current_player;
            rendered = true;
        }

        ui_render_flags_clear(flags, RENDER_CURRENT_TURN);
    }

    return rendered;
}
