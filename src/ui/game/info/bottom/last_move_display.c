#include "last_move_display.h"
#include "../../../core/theme.h"
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <wchar.h>

typedef struct
{
    const char *l1;
    const char *l2;
    const char *l3;
} Glyph3;

Glyph3 ROWS_ASCII[] = {
    {"┏━━┓",
     "┣━━┫",
     "╹  ╹"}, // A
    {"╻   ",
     "┣━━┓",
     "┗━━┛"}, // B
    {"┏━━╸",
     "┃   ",
     "┗━━╸"}, // C
    {"   ╻",
     "┏━━┫",
     "┗━━┛"}, // D
    {"┏━━╸",
     "┣━━╸",
     "┗━━╸"}, // E
    {"┏━━╸",
     "┣━━╸",
     "╹   "}, // F
    {"┏━━╸",
     "┃╺━┓",
     "┗━━┛"}, // G
    {"╻  ╻",
     "┣━━┫",
     "╹  ╹"}, // H
    // I 없음
    {"╺━┳╸",
     "  ┃ ",
     "┗━┛ "}, // J
    {"╻ ┏╸",
     "┣┫  ",
     "╹ ┗╸"}, // K
    {"╻   ",
     "┃   ",
     "┗━━╸"}, // L
    {"┏┓┏┓",
     "┃┃┃┃",
     "╹┗┛╹"}, // M
    {"┏┓ ╻",
     "┃┗┓┃",
     "╹ ┗┛"}, // N
    {"┏━━┓",
     "┃  ┃",
     "┗━━┛"}, // O
    {"┏━━┓",
     "┣━━┛",
     "╹   "}, // P
    {"┏━━┓",
     "┗━━┫",
     "   ┛"}, // Q
    {"┏━━┓",
     "┣━┳┛",
     "╹ ┗╸"}, // R
    {"┏━━┓",
     "┗━━┓",
     "┗━━┛"}, // S
    {" ━┳━",
     "  ┃ "
     "  ╹ "}, // T
};

Glyph3 COLS_ASCII[] = {
    {"┏━━┓",
     "┃  ┃",
     "┗━━┛"}, // 0
    {" ━┓ ",
     "  ┃ ",
     "╺━┻━"}, // 1
    {"╺━━┓",
     "┏━━┛",
     "┗━━╸"}, // 2
    {"╺━━┓",
     "╺━━┫",
     "╺━━┛"}, // 3
    {"╻  ╻",
     "┗━━┫",
     "   ╹"}, // 4
    {"┏━━╸",
     "┗━━┓",
     "┗━━┛"}, // 5
    {"┏━━╸",
     "┣━━┓",
     "┗━━┛"}, // 6
    {"╺━━┓",
     "   ┃",
     "   ╹"}, // 7
    {"┏━━┓",
     "┣━━┫",
     "┗━━┛"}, // 8
    {"┏━━┓",
     "┗━━┫",
     "╺━━┛"}, // 9
};

void extract_ascii_char(const char *line, int index, char *out)
{
    // index: 0-based, 각 토큰은 원래 4바이트 블록으로 설계되어 있음.
    // UTF-8 멀티바이트 문자를 중간에서 자르지 않도록 mbrtowc를 사용해
    // 유효한 문자 경계까지만 복사합니다.
    int start = index * 4;
    const char *p = line + start;
    size_t max_bytes = 4;

    mbstate_t st;
    memset(&st, 0, sizeof(st));

    size_t consumed = 0;
    const char *q = p;
    while (consumed < max_bytes && *q != '\0')
    {
        wchar_t wc;
        size_t len = mbrtowc(&wc, q, max_bytes - consumed, &st);
        if (len == (size_t)-1 || len == (size_t)-2)
        {
            // invalid or incomplete sequence: stop before this byte
            break;
        }
        if (len == 0)
        {
            // NUL encountered
            break;
        }
        consumed += len;
        q += len;
    }

    if (consumed == 0)
    {
        // fallback: copy up to max_bytes (will likely produce valid output)
        consumed = strnlen(p, max_bytes);
    }

    strncpy(out, p, consumed);
    out[consumed] = '\0';
}

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
    // 내부 좌표: (1,25) ~ (17,31)

    // 마지막 수의 돌 색상에 따라 색상 적용
    Stone last_stone = EMPTY;
    if (last_move.row != -1 && last_move.col != -1)
    {
        last_stone = board_get_stone(board, last_move.row, last_move.col);
    }

    attron(COLOR_PAIR(COLOR_PAIR_DEFAULT) | A_BOLD);
    mvprintw(25, 5, "LAST MOVE");
    mvprintw(26, 0, "┠─────────────────");
    attroff(COLOR_PAIR(COLOR_PAIR_DEFAULT) | A_BOLD);

    if (last_move.row == -1 || last_move.col == -1)
    {
        // 아직 수가 없음
        int begy, begx;
        getbegyx(win, begy, begx);
        int local_y = 27 - begy;
        int local_x = 3 - begx;

        wattron(win, COLOR_PAIR(COLOR_PAIR_DEFAULT) | A_BOLD);
        mvwprintw(win, local_y + 0, local_x, "┏┓ ╻ ╻  ╻╻     ┃");
        mvwprintw(win, local_y + 1, local_x, "┃┗┓┃ ┃  ┃┃     ┃");
        mvwprintw(win, local_y + 2, local_x, "╹ ┗┛ ┗━━┛┗━━╸  ┃");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DEFAULT) | A_BOLD);
    }
    else
    {
        int col_idx = last_move.col;
        int row = last_move.row + 1;
        int row10 = row / 10;
        int row1 = row % 10;

        Glyph3 r = ROWS_ASCII[col_idx];
        Glyph3 c1 = COLS_ASCII[row10];
        Glyph3 c2 = COLS_ASCII[row1];

        int y = 27;
        int x = 3;

        attron(COLOR_PAIR(COLOR_PAIR_DEFAULT) | A_BOLD);
        mvprintw(y + 0, x, "%s %s%s", r.l1, c1.l1, c2.l1);
        mvprintw(y + 1, x, "%s %s%s", r.l2, c1.l2, c2.l2);
        mvprintw(y + 2, x, "%s %s%s", r.l3, c1.l3, c2.l3);
        attroff(COLOR_PAIR(COLOR_PAIR_DEFAULT) | A_BOLD);
    }
    wrefresh(win);
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
