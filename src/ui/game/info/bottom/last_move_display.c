#include "last_move_display.h"
#include <stdio.h>

void last_move_display_init(LastMoveDisplay *display)
{
    if (!display)
        return;
    display->prev_row = -1;
    display->prev_col = -1;
}

void last_move_display_draw(WINDOW *win, const Board *board)
{
    if (!win || !board)
        return;

    Position last_move = board_get_last_move(board);

    // 제목은 ingame_border에서 그리므로 여기서는 내용만
    // LAST MOVE 박스: (0,24) 시작, 19x7
    // 내부 좌표: (1,25) ~ (17,30)

    if (last_move.row == -1 || last_move.col == -1)
    {
        // 아직 수가 없음
        mvprintw(27, 2, "  ----  ----  ");
        mvprintw(28, 2, "              ");
        mvprintw(29, 2, "              ");
    }
    else
    {
        Stone last_stone = board_get_stone(board, last_move.row, last_move.col);

        // Draw stone type and position
        int x = 2;
        if (last_stone == BLACK)
        {
            mvprintw(27, x, "╻    ╻ ┏╸    ");
            mvprintw(28, x, "┣━━┓ ┣┫      ");
            mvprintw(29, x, "┗━━┛ ╹ ┗╸    ");
        }
        else
        {
            mvprintw(27, x, "╻┏┓╻ ━┳━     ");
            mvprintw(28, x, "┃┃┃┃  ┃      ");
            mvprintw(29, x, "┗┛┗┛  ╹      ");
        }
    }
    refresh();
}

bool last_move_display_render(WINDOW *win, const Board *board,
                              LastMoveDisplay *display,
                              UIRenderFlags *flags,
                              bool force_render)
{
    if (!win || !board || !display || !flags)
        return false;

    bool rendered = false;

    if (force_render || ui_render_flags_is_set(flags, RENDER_LAST_MOVE))
    {
        Position last_move = board_get_last_move(board);

        // 변경 여부 확인
        if (force_render ||
            display->prev_row != last_move.row ||
            display->prev_col != last_move.col)
        {
            last_move_display_draw(win, board);

            display->prev_row = last_move.row;
            display->prev_col = last_move.col;
            rendered = true;
        }

        ui_render_flags_clear(flags, RENDER_LAST_MOVE);
    }

    return rendered;
}
