#ifndef LAST_MOVE_DISPLAY_H
#define LAST_MOVE_DISPLAY_H

#include "../../../../game/core/board.h"
#include "../../../core/ui_manager.h"
#include <ncurses.h>
#include <stdbool.h>

// 마지막 수 표시 상태 추적
typedef struct
{
    int prev_row;
    int prev_col;
} LastMoveDisplay;

// 초기화
void last_move_display_init(LastMoveDisplay *display);

// 그리기 (항상 그림)
void last_move_display_draw(WINDOW *win, const Board *board);

// 선택적 렌더링 (dirty flag 기반)
// 반환값: 렌더링이 발생했으면 true
bool last_move_display_render(WINDOW *win, const Board *board,
                              LastMoveDisplay *display,
                              UIRenderFlags *flags,
                              bool force_render);

#endif // LAST_MOVE_DISPLAY_H
