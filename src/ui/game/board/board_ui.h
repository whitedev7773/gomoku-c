#ifndef BOARD_UI_H
#define BOARD_UI_H

#include "../../../game/core/board.h"
#include "../../core/ui_manager.h"
#include <ncurses.h>

typedef struct
{
    int cursor_row;
    int cursor_col;
    int prev_cursor_row; // 이전 커서 위치 (최적화용)
    int prev_cursor_col;
} BoardCursor;

void board_init_cursor(BoardCursor *cursor);

// 초기 렌더링 (테두리 포함 전체)
void board_init_win(WINDOW *win);

// 전체 렌더링 (호환성을 위해 유지)
void board_render_full(WINDOW *win, const Board *board, const BoardCursor *cursor);

// 최적화된 업데이트 (변경된 부분만)
void board_update(WINDOW *win, const Board *board, const BoardCursor *cursor);

// 특정 셀만 다시 그리기
void board_redraw_cell(WINDOW *win, const Board *board, const BoardCursor *cursor, int row, int col);

void board_draw_border(WINDOW *win);

void board_draw_coords(WINDOW *win);

void board_draw(WINDOW *win, const Board *board, const BoardCursor *cursor);

void board_move_cursor(BoardCursor *cursor, int dr, int dc);

char board_col_to_char(int col);

int board_char_to_col(char c);

// ============================================
// 선택적 렌더링 함수 (Dirty Flag 기반)
// ============================================

// 선택적 렌더링 (dirty flag 기반)
// first_render가 true면 테두리 포함 전체 렌더링
// 이후에는 변경된 부분만 렌더링
void board_render(WINDOW *win, const Board *board,
                               const BoardCursor *cursor,
                               UIRenderFlags *flags,
                               bool first_render);

// 멀티플레이용 선택적 렌더링 (상대방 커서 포함)
// is_my_turn이 false면 내 커서를 숨김
void board_render_mp(WINDOW *win, const Board *board,
                                           const BoardCursor *my_cursor,
                                           const BoardCursor *opponent_cursor,
                                           UIRenderFlags *flags,
                                           bool first_render,
                                           bool is_my_turn);

// 커서 이동 시 dirty 셀 마킹
void board_move_cursor_f(BoardCursor *cursor, int dr, int dc,
                                     UIRenderFlags *flags);

// 상대방 커서 위치 업데이트 (dirty flag 설정)
void board_update_opponent_cursor(BoardCursor *opponent_cursor, int row, int col,
                                     UIRenderFlags *flags);

#endif // BOARD_UI_H
