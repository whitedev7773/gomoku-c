#ifndef NOW_TURN_DISPLAY_H
#define NOW_TURN_DISPLAY_H

#include "../../../../game/core/board.h"
#include "../../../../game/core/turn_manager.h"
#include "../../../core/ui_manager.h"
#include <ncurses.h>
#include <stdbool.h>

// 현재 턴 표시 상태 추적
typedef struct
{
    Stone prev_current_player;
} NowTurnDisplay;

// 초기화
void now_turn_display_init(NowTurnDisplay *display);

// 그리기 (항상 그림)
void now_turn_display_draw(WINDOW *win, Stone current_player);

// 선택적 렌더링 (dirty flag 기반)
// 반환값: 렌더링이 발생했으면 true
bool now_turn_display_render(WINDOW *win, const TurnManager *turn_mgr,
                             NowTurnDisplay *display,
                             UIRenderFlags *flags,
                             bool force_render);

#endif // NOW_TURN_DISPLAY_H
