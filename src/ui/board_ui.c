#include "board_ui.h"
#include "theme.h"
#include <string.h>

// Column labels (A-T, excluding I)
static const char COL_LABELS[] = "ABCDEFGHJKLMNOPQRST";

void board_ui_init_cursor(BoardCursor *cursor)
{
    if (!cursor)
        return;
    cursor->cursor_row = 9;
    cursor->cursor_col = 9;
    cursor->prev_cursor_row = 9;
    cursor->prev_cursor_col = 9;
}

char board_ui_col_to_char(int col)
{
    if (col < 0 || col >= BOARD_SIZE)
        return '?';
    return COL_LABELS[col];
}

int board_ui_char_to_col(char c)
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        if (COL_LABELS[i] == c)
            return i;
    }
    return -1;
}

void board_ui_draw_border(WINDOW *win)
{
    // ingame_border.c가 stdscr에 테두리를 그리므로
    // 여기서는 좌표 레이블과 보드 프레임만 그림
    // Board 영역: ingame_border (0,0)~(48,24) = 49x25
    // board_win은 (1,1)~(47,23) = 47x23 크기

    // 상단 좌표 레이블 (A-T)
    mvwprintw(win, 0, 4, " ");
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        wprintw(win, "%c ", COL_LABELS[i]);
    }

    // 보드 프레임 상단
    mvwprintw(win, 1, 2, "┌");
    for (int i = 0; i < 39; i++)
    {
        wprintw(win, "─");
    }
    wprintw(win, "┐");

    // 행 번호와 세로선
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        int y = 2 + row;
        mvwprintw(win, y, 0, "%2d│", BOARD_SIZE - row);
        mvwprintw(win, y, 42, "│%-2d", BOARD_SIZE - row);
    }

    // 보드 프레임 하단
    mvwprintw(win, 21, 2, "└");
    for (int i = 0; i < 39; i++)
    {
        wprintw(win, "─");
    }
    wprintw(win, "┘");

    // 하단 좌표 레이블 (A-T)
    mvwprintw(win, 22, 4, " ");
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        wprintw(win, "%c ", COL_LABELS[i]);
    }
}

void board_ui_draw_board(WINDOW *win, const Board *board, const BoardCursor *cursor)
{
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        // board_win 내부 좌표: 보드 시작 y=2 (프레임 상단 아래), x=4 (행번호+│ 뒤)
        int y = 2 + row;
        int x = 4;

        wmove(win, y, x);

        for (int col = 0; col < BOARD_SIZE; col++)
        {
            Stone stone = board_get_stone(board, row, col);

            bool is_cursor = (cursor && cursor->cursor_row == row && cursor->cursor_col == col);
            bool is_last_move = (board->last_row == row && board->last_col == col);

            // 커서 위치면 '[' 출력
            if (is_cursor)
            {
                int cur_y, cur_x;
                getyx(win, cur_y, cur_x);
                // 첫 번째 열이라도 x-1 위치에 '[' 출력
                mvwaddch(win, cur_y, cur_x - 1, '[');
                wmove(win, cur_y, cur_x);
            }

            if (stone == BLACK)
            {
                if (is_last_move)
                    wattron(win, COLOR_PAIR(COLOR_PAIR_BLACK_STONE) | A_BOLD);
                wprintw(win, "●");
                if (is_last_move)
                    wattroff(win, COLOR_PAIR(COLOR_PAIR_BLACK_STONE) | A_BOLD);
            }
            else if (stone == WHITE)
            {
                if (is_last_move)
                    wattron(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_BOLD);
                wprintw(win, "○");
                if (is_last_move)
                    wattroff(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_BOLD);
            }
            else
            {
                // 빈 칸: 금수 위치 확인
                if (board_is_forbidden(board, row, col))
                {
                    wattron(win, COLOR_PAIR(COLOR_PAIR_SYSTEM) | A_BOLD);
                    wprintw(win, "x");
                    wattroff(win, COLOR_PAIR(COLOR_PAIR_SYSTEM) | A_BOLD);
                }
                else
                {
                    wprintw(win, "·");
                }
            }

            // 커서 위치면 ']' 출력, 아니면 공백
            if (col < BOARD_SIZE - 1)
            {
                if (is_cursor)
                {
                    waddch(win, ']');
                }
                else
                {
                    waddch(win, ' ');
                }
            }
            else if (is_cursor)
            {
                // 마지막 열이면서 커서인 경우
                waddch(win, ']');
            }
        }
    }
}

void board_ui_render(WINDOW *win, const Board *board, const BoardCursor *cursor)
{
    if (!win)
        return;

    wclear(win);
    board_ui_draw_border(win);

    if (board)
    {
        board_ui_draw_board(win, board, cursor);
    }

    wrefresh(win);
}

void board_ui_move_cursor(BoardCursor *cursor, int dr, int dc)
{
    if (!cursor)
        return;

    // 이전 위치 저장 (최적화용)
    cursor->prev_cursor_row = cursor->cursor_row;
    cursor->prev_cursor_col = cursor->cursor_col;

    int new_row = cursor->cursor_row + dr;
    int new_col = cursor->cursor_col + dc;

    if (new_row >= 0 && new_row < BOARD_SIZE)
    {
        cursor->cursor_row = new_row;
    }

    if (new_col >= 0 && new_col < BOARD_SIZE)
    {
        cursor->cursor_col = new_col;
    }
}

// 초기 렌더링 (테두리 포함 전체 - 한 번만 호출)
void board_ui_init(WINDOW *win)
{
    if (!win)
        return;

    wclear(win);
    board_ui_draw_border(win);
    wrefresh(win);
}

// 특정 셀만 다시 그리기 (최적화)
void board_ui_redraw_cell(WINDOW *win, const Board *board, const BoardCursor *cursor, int row, int col)
{
    if (!win || !board || row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE)
        return;

    // board_win 내부 좌표에 맞춤
    int y = 2 + row;
    int x = 4 + (col * 2);

    Stone stone = board_get_stone(board, row, col);
    bool is_cursor = (cursor && cursor->cursor_row == row && cursor->cursor_col == col);
    bool is_last_move = (board->last_row == row && board->last_col == col);

    // 왼쪽 셀이 커서인지 확인 (현재 셀의 x-1 위치에 ']'를 출력해야 함)
    bool left_is_cursor = (cursor && cursor->cursor_row == row && cursor->cursor_col == col - 1);

    // x-1 위치 처리: 커서면 '[', 왼쪽 셀이 커서면 ']', 아니면 공백
    if (is_cursor)
    {
        mvwaddch(win, y, x - 1, '[');
    }
    else if (left_is_cursor)
    {
        mvwaddch(win, y, x - 1, ']');
    }
    else
    {
        mvwaddch(win, y, x - 1, ' ');
    }

    wmove(win, y, x);

    if (stone == BLACK)
    {
        if (is_last_move)
            wattron(win, COLOR_PAIR(COLOR_PAIR_BLACK_STONE) | A_BOLD);
        wprintw(win, "●");
        if (is_last_move)
            wattroff(win, COLOR_PAIR(COLOR_PAIR_BLACK_STONE) | A_BOLD);
    }
    else if (stone == WHITE)
    {
        if (is_last_move)
            wattron(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_BOLD);
        wprintw(win, "○");
        if (is_last_move)
            wattroff(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_BOLD);
    }
    else
    {
        // 빈 칸: 금수 위치 확인
        if (board_is_forbidden(board, row, col))
        {
            wattron(win, COLOR_PAIR(COLOR_PAIR_SYSTEM) | A_BOLD);
            wprintw(win, "x");
            wattroff(win, COLOR_PAIR(COLOR_PAIR_SYSTEM) | A_BOLD);
        }
        else
        {
            wprintw(win, "·");
        }
    }

    // 커서면 ']' 출력, 오른쪽 셀이 커서면 '[' 출력, 아니면 공백
    bool right_is_cursor = (cursor && cursor->cursor_row == row && cursor->cursor_col == col + 1);

    if (col < BOARD_SIZE - 1)
    {
        if (is_cursor)
        {
            waddch(win, ']');
        }
        else if (right_is_cursor)
        {
            waddch(win, '[');
        }
        else
        {
            waddch(win, ' ');
        }
    }
    else if (is_cursor)
    {
        waddch(win, ']');
    }
}

// 최적화된 업데이트 (변경된 부분만)
void board_ui_update(WINDOW *win, const Board *board, const BoardCursor *cursor)
{
    if (!win || !board)
        return;

    // 1. 이전 커서 위치 지우기 (이전 위치만)
    if (cursor && (cursor->prev_cursor_row != cursor->cursor_row ||
                   cursor->prev_cursor_col != cursor->cursor_col))
    {
        board_ui_redraw_cell(win, board, NULL, cursor->prev_cursor_row, cursor->prev_cursor_col);
    }

    // 2. 현재 커서 위치 그리기
    if (cursor)
    {
        board_ui_redraw_cell(win, board, cursor, cursor->cursor_row, cursor->cursor_col);
    }

    // 3. 마지막 수 위치 업데이트 (항상 강조 표시)
    if (board->last_row >= 0 && board->last_col >= 0)
    {
        board_ui_redraw_cell(win, board, cursor, board->last_row, board->last_col);
    }

    wrefresh(win);
}

// ============================================
// 선택적 렌더링 함수 (Dirty Flag 기반)
// ============================================

void board_ui_selective_render(WINDOW *win, const Board *board,
                               const BoardCursor *cursor,
                               UIRenderFlags *flags,
                               bool first_render)
{
    if (!win || !board || !flags)
        return;

    bool need_refresh = false;

    // 첫 렌더링 또는 전체 보드 렌더링 요청
    if (first_render || ui_render_flags_is_set(flags, RENDER_BOARD_FULL))
    {
        wclear(win);
        board_ui_draw_border(win);
        board_ui_draw_board(win, board, cursor);
        need_refresh = true;
        ui_render_flags_clear(flags, RENDER_BOARD_FULL);
    }
    else
    {
        // 커서 이동만 있는 경우
        if (ui_render_flags_is_set(flags, RENDER_BOARD_CURSOR))
        {
            // 이전 커서 위치 지우기
            if (cursor && (cursor->prev_cursor_row != cursor->cursor_row ||
                           cursor->prev_cursor_col != cursor->cursor_col))
            {
                board_ui_redraw_cell(win, board, NULL, cursor->prev_cursor_row, cursor->prev_cursor_col);
            }

            // 현재 커서 위치 그리기
            if (cursor)
            {
                board_ui_redraw_cell(win, board, cursor, cursor->cursor_row, cursor->cursor_col);
            }
            need_refresh = true;
            ui_render_flags_clear(flags, RENDER_BOARD_CURSOR);
        }

        // 특정 셀만 업데이트
        if (ui_render_flags_is_set(flags, RENDER_BOARD_CELL))
        {
            for (int i = 0; i < flags->dirty_cell_count; i++)
            {
                int row = flags->dirty_cells[i][0];
                int col = flags->dirty_cells[i][1];
                board_ui_redraw_cell(win, board, cursor, row, col);
            }
            ui_render_flags_clear_dirty_cells(flags);
            ui_render_flags_clear(flags, RENDER_BOARD_CELL);
            need_refresh = true;
        }
    }

    // 마지막 수 항상 강조 (새로운 돌이 놓였을 때)
    if (board->last_row >= 0 && board->last_col >= 0)
    {
        board_ui_redraw_cell(win, board, cursor, board->last_row, board->last_col);
        need_refresh = true;
    }

    if (need_refresh)
    {
        wrefresh(win);
    }
}

void board_ui_move_cursor_with_flags(BoardCursor *cursor, int dr, int dc,
                                     UIRenderFlags *flags)
{
    if (!cursor || !flags)
        return;

    // 이전 위치 저장
    cursor->prev_cursor_row = cursor->cursor_row;
    cursor->prev_cursor_col = cursor->cursor_col;

    int new_row = cursor->cursor_row + dr;
    int new_col = cursor->cursor_col + dc;

    if (new_row >= 0 && new_row < BOARD_SIZE)
    {
        cursor->cursor_row = new_row;
    }

    if (new_col >= 0 && new_col < BOARD_SIZE)
    {
        cursor->cursor_col = new_col;
    }

    // 커서가 실제로 이동했으면 dirty flag 설정
    if (cursor->prev_cursor_row != cursor->cursor_row ||
        cursor->prev_cursor_col != cursor->cursor_col)
    {
        ui_render_flags_set(flags, RENDER_BOARD_CURSOR);
    }
}

// 상대방 커서 위치 업데이트
void board_ui_update_opponent_cursor(BoardCursor *opponent_cursor, int row, int col,
                                     UIRenderFlags *flags)
{
    if (!opponent_cursor || !flags)
        return;

    // 이전 위치 저장
    opponent_cursor->prev_cursor_row = opponent_cursor->cursor_row;
    opponent_cursor->prev_cursor_col = opponent_cursor->cursor_col;

    // 새 위치 설정
    opponent_cursor->cursor_row = row;
    opponent_cursor->cursor_col = col;

    // 커서가 실제로 이동했으면 dirty flag 설정
    if (opponent_cursor->prev_cursor_row != opponent_cursor->cursor_row ||
        opponent_cursor->prev_cursor_col != opponent_cursor->cursor_col)
    {
        ui_render_flags_set(flags, RENDER_BOARD_CURSOR);
    }
}

// 특정 셀 그리기 (멀티플레이용 - 내 커서와 상대방 커서 모두 고려)
static void board_ui_redraw_cell_multiplayer(WINDOW *win, const Board *board,
                                             const BoardCursor *my_cursor,
                                             const BoardCursor *opponent_cursor,
                                             int row, int col)
{
    if (!win || !board || row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE)
        return;

    // board_win 내부 좌표에 맞춤
    int y = 2 + row;
    int x = 4 + (col * 2);

    Stone stone = board_get_stone(board, row, col);
    bool is_my_cursor = (my_cursor && my_cursor->cursor_row == row && my_cursor->cursor_col == col);
    bool is_opponent_cursor = (opponent_cursor && opponent_cursor->cursor_row == row && opponent_cursor->cursor_col == col);
    bool is_last_move = (board->last_row == row && board->last_col == col);

    // 왼쪽 셀이 커서인지 확인
    bool left_is_my_cursor = (my_cursor && my_cursor->cursor_row == row && my_cursor->cursor_col == col - 1);
    bool left_is_opponent_cursor = (opponent_cursor && opponent_cursor->cursor_row == row && opponent_cursor->cursor_col == col - 1);

    // x-1 위치 처리: 커서면 '[', 왼쪽 셀이 커서면 ']', 아니면 공백
    {
        if (is_my_cursor)
        {
            mvwaddch(win, y, x - 1, '[');
        }
        else if (is_opponent_cursor)
        {
            wattron(win, A_UNDERLINE | COLOR_PAIR(6));
            mvwaddch(win, y, x - 1, '[');
            wattroff(win, A_UNDERLINE | COLOR_PAIR(6));
        }
        else if (left_is_my_cursor)
        {
            mvwaddch(win, y, x - 1, ']');
        }
        else if (left_is_opponent_cursor)
        {
            wattron(win, A_UNDERLINE | COLOR_PAIR(6));
            mvwaddch(win, y, x - 1, ']');
            wattroff(win, A_UNDERLINE | COLOR_PAIR(6));
        }
        else
        {
            mvwaddch(win, y, x - 1, ' ');
        }
    }

    wmove(win, y, x);

    // 상대방 커서: 밑줄 + 파란색
    if (is_opponent_cursor && !is_my_cursor)
    {
        wattron(win, A_UNDERLINE | COLOR_PAIR(6));
    }

    if (stone == BLACK)
    {
        if (is_last_move)
            wattron(win, COLOR_PAIR(COLOR_PAIR_BLACK_STONE) | A_BOLD);
        wprintw(win, "●");
        if (is_last_move)
            wattroff(win, COLOR_PAIR(COLOR_PAIR_BLACK_STONE) | A_BOLD);
    }
    else if (stone == WHITE)
    {
        if (is_last_move)
            wattron(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_BOLD);
        wprintw(win, "○");
        if (is_last_move)
            wattroff(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_BOLD);
    }
    else
    {
        // 빈 칸: 금수 위치 확인
        if (board_is_forbidden(board, row, col))
        {
            wattron(win, COLOR_PAIR(COLOR_PAIR_SYSTEM) | A_BOLD);
            wprintw(win, "x");
            wattroff(win, COLOR_PAIR(COLOR_PAIR_SYSTEM) | A_BOLD);
        }
        else
        {
            wprintw(win, "·");
        }
    }

    // 커서면 ']' 출력, 오른쪽 셀이 커서면 '[', 아니면 공백
    bool right_is_my_cursor = (my_cursor && my_cursor->cursor_row == row && my_cursor->cursor_col == col + 1);
    bool right_is_opponent_cursor = (opponent_cursor && opponent_cursor->cursor_row == row && opponent_cursor->cursor_col == col + 1);

    if (col < BOARD_SIZE - 1)
    {
        if (is_my_cursor)
        {
            waddch(win, ']');
        }
        else if (is_opponent_cursor)
        {
            waddch(win, ']');
        }
        else if (right_is_my_cursor)
        {
            waddch(win, '[');
        }
        else if (right_is_opponent_cursor)
        {
            wattron(win, A_UNDERLINE | COLOR_PAIR(6));
            waddch(win, '[');
            wattroff(win, A_UNDERLINE | COLOR_PAIR(6));
        }
        else
        {
            waddch(win, ' ');
        }
    }
    else if (is_my_cursor || is_opponent_cursor)
    {
        waddch(win, ']');
    }

    if (is_opponent_cursor && !is_my_cursor)
    {
        wattroff(win, A_UNDERLINE | COLOR_PAIR(6));
    }
}

// 보드 전체 그리기 (멀티플레이용)
static void board_ui_draw_board_multiplayer(WINDOW *win, const Board *board,
                                            const BoardCursor *my_cursor,
                                            const BoardCursor *opponent_cursor)
{
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        // board_win 내부 좌표에 맞춤
        int y = 2 + row;
        int x = 4;

        wmove(win, y, x);

        for (int col = 0; col < BOARD_SIZE; col++)
        {
            Stone stone = board_get_stone(board, row, col);

            bool is_my_cursor = (my_cursor && my_cursor->cursor_row == row && my_cursor->cursor_col == col);
            bool is_opponent_cursor = (opponent_cursor && opponent_cursor->cursor_row == row && opponent_cursor->cursor_col == col);
            bool is_last_move = (board->last_row == row && board->last_col == col);

            // 커서 위치면 '[' 출력 (첫 열이라도 표시)
            if (is_my_cursor || is_opponent_cursor)
            {
                int cur_y, cur_x;
                getyx(win, cur_y, cur_x);
                if (is_opponent_cursor && !is_my_cursor)
                    wattron(win, A_UNDERLINE | COLOR_PAIR(6));
                mvwaddch(win, cur_y, cur_x - 1, '[');
                if (is_opponent_cursor && !is_my_cursor)
                    wattroff(win, A_UNDERLINE | COLOR_PAIR(6));
                wmove(win, cur_y, cur_x);
            }

            // 상대방 커서: 밑줄 + 파란색
            if (is_opponent_cursor && !is_my_cursor)
            {
                wattron(win, A_UNDERLINE | COLOR_PAIR(6));
            }

            if (stone == BLACK)
            {
                if (is_last_move)
                    wattron(win, COLOR_PAIR(COLOR_PAIR_BLACK_STONE) | A_BOLD);
                wprintw(win, "●");
                if (is_last_move)
                    wattroff(win, COLOR_PAIR(COLOR_PAIR_BLACK_STONE) | A_BOLD);
            }
            else if (stone == WHITE)
            {
                if (is_last_move)
                    wattron(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_BOLD);
                wprintw(win, "○");
                if (is_last_move)
                    wattroff(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_BOLD);
            }
            else
            {
                // 빈 칸: 금수 위치 확인
                if (board_is_forbidden(board, row, col))
                {
                    wattron(win, COLOR_PAIR(COLOR_PAIR_SYSTEM) | A_BOLD);
                    wprintw(win, "x");
                    wattroff(win, COLOR_PAIR(COLOR_PAIR_SYSTEM) | A_BOLD);
                }
                else
                {
                    wprintw(win, "·");
                }
            }

            if (is_opponent_cursor && !is_my_cursor)
            {
                wattroff(win, A_UNDERLINE | COLOR_PAIR(6));
            }

            // 커서 위치면 ']' 출력, 아니면 공백
            if (col < BOARD_SIZE - 1)
            {
                if (is_my_cursor)
                {
                    waddch(win, ']');
                }
                else if (is_opponent_cursor)
                {
                    wattron(win, A_UNDERLINE | COLOR_PAIR(6));
                    waddch(win, ']');
                    wattroff(win, A_UNDERLINE | COLOR_PAIR(6));
                }
                else
                {
                    waddch(win, ' ');
                }
            }
            else if (is_my_cursor || is_opponent_cursor)
            {
                if (is_opponent_cursor && !is_my_cursor)
                    wattron(win, A_UNDERLINE | COLOR_PAIR(6));
                waddch(win, ']');
                if (is_opponent_cursor && !is_my_cursor)
                    wattroff(win, A_UNDERLINE | COLOR_PAIR(6));
            }
        }
    }
}

// 멀티플레이용 선택적 렌더링 (상대방 커서 포함)
void board_ui_selective_render_multiplayer(WINDOW *win, const Board *board,
                                           const BoardCursor *my_cursor,
                                           const BoardCursor *opponent_cursor,
                                           UIRenderFlags *flags,
                                           bool first_render,
                                           bool is_my_turn)
{
    if (!win || !board || !flags)
        return;

    // 내 턴: 내 커서만 표시, 상대 커서 숨김
    // 상대 턴: 상대 커서만 표시, 내 커서 숨김
    const BoardCursor *effective_my_cursor = is_my_turn ? my_cursor : NULL;
    const BoardCursor *effective_opponent_cursor = is_my_turn ? NULL : opponent_cursor;

    bool need_refresh = false;

    // 첫 렌더링 또는 전체 보드 렌더링 요청
    if (first_render || ui_render_flags_is_set(flags, RENDER_BOARD_FULL))
    {
        wclear(win);
        board_ui_draw_border(win);
        board_ui_draw_board_multiplayer(win, board, effective_my_cursor, effective_opponent_cursor);
        need_refresh = true;
        ui_render_flags_clear(flags, RENDER_BOARD_FULL);
    }
    else
    {
        // 커서 이동만 있는 경우
        if (ui_render_flags_is_set(flags, RENDER_BOARD_CURSOR))
        {
            // 내 커서 이전 위치 지우기 (내 턴일 때만)
            if (effective_my_cursor && (my_cursor->prev_cursor_row != my_cursor->cursor_row ||
                                        my_cursor->prev_cursor_col != my_cursor->cursor_col))
            {
                board_ui_redraw_cell_multiplayer(win, board, effective_my_cursor, effective_opponent_cursor,
                                                 my_cursor->prev_cursor_row, my_cursor->prev_cursor_col);
            }

            // 상대방 커서 이전 위치 지우기 (상대 턴일 때만)
            if (effective_opponent_cursor && (opponent_cursor->prev_cursor_row != opponent_cursor->cursor_row ||
                                              opponent_cursor->prev_cursor_col != opponent_cursor->cursor_col))
            {
                board_ui_redraw_cell_multiplayer(win, board, effective_my_cursor, effective_opponent_cursor,
                                                 opponent_cursor->prev_cursor_row, opponent_cursor->prev_cursor_col);
            }

            // 내 커서 현재 위치 그리기 (내 턴일 때만)
            if (effective_my_cursor)
            {
                board_ui_redraw_cell_multiplayer(win, board, effective_my_cursor, effective_opponent_cursor,
                                                 my_cursor->cursor_row, my_cursor->cursor_col);
            }

            // 상대방 커서 현재 위치 그리기 (상대 턴일 때만)
            if (effective_opponent_cursor)
            {
                board_ui_redraw_cell_multiplayer(win, board, effective_my_cursor, effective_opponent_cursor,
                                                 opponent_cursor->cursor_row, opponent_cursor->cursor_col);
            }

            need_refresh = true;
            ui_render_flags_clear(flags, RENDER_BOARD_CURSOR);
        }

        // 특정 셀만 업데이트
        if (ui_render_flags_is_set(flags, RENDER_BOARD_CELL))
        {
            for (int i = 0; i < flags->dirty_cell_count; i++)
            {
                int row = flags->dirty_cells[i][0];
                int col = flags->dirty_cells[i][1];
                board_ui_redraw_cell_multiplayer(win, board, effective_my_cursor, effective_opponent_cursor, row, col);
            }
            ui_render_flags_clear_dirty_cells(flags);
            ui_render_flags_clear(flags, RENDER_BOARD_CELL);
            need_refresh = true;
        }
    }

    // 마지막 수 항상 강조 (새로운 돌이 놓였을 때)
    if (board->last_row >= 0 && board->last_col >= 0)
    {
        board_ui_redraw_cell_multiplayer(win, board, effective_my_cursor, effective_opponent_cursor,
                                         board->last_row, board->last_col);
        need_refresh = true;
    }

    if (need_refresh)
    {
        wrefresh(win);
    }
}
