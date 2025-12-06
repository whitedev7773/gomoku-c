#ifndef TIME_DISPLAY_H
#define TIME_DISPLAY_H

#include "../../../../game/core/turn_manager.h"
#include "../../../core/ui_manager.h"
#include <ncurses.h>
#include <stdbool.h>
#include <time.h>

// 시간 표시 상태 추적
typedef struct
{
    time_t game_start_time;     // 게임 시작 시간
    int prev_remaining_seconds; // 이전 턴 남은 시간
    int prev_elapsed_seconds;   // 이전 경과 시간
} TimeDisplay;

// 초기화
void time_display_init(TimeDisplay *display);

// 경과 시간 계산
int time_display_elapsed_sec(const TimeDisplay *display);

// 턴 타이머 그리기
void time_display_draw_timer(WINDOW *win, int remaining_seconds);

// 플레이 시간 그리기
void time_display_draw_playtime(WINDOW *win, const TimeDisplay *display);

// 선택적 렌더링 - 타이머 (dirty flag 기반)
// 반환값: 렌더링이 발생했으면 true
bool time_display_render_timer(WINDOW *win, const TurnManager *turn_mgr,
                               TimeDisplay *display,
                               UIRenderFlags *flags,
                               bool force_render);

// 선택적 렌더링 - 플레이 시간 (dirty flag 기반)
// 반환값: 렌더링이 발생했으면 true
bool time_display_render_playtime(WINDOW *win,
                                  TimeDisplay *display,
                                  UIRenderFlags *flags,
                                  bool force_render);

#endif // TIME_DISPLAY_H
