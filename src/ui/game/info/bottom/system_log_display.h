#ifndef SYSTEM_LOG_DISPLAY_H
#define SYSTEM_LOG_DISPLAY_H

#include "../../../core/ui_manager.h"
#include <ncurses.h>
#include <stdbool.h>

// 시스템 로그 최대 라인 수 (SYSTEM LOG 박스 높이 - 테두리)
#define SYSTEM_LOG_MAX_LINES 3
#define SYSTEM_LOG_MAX_LENGTH 64

// 시스템 로그 표시 상태 추적
typedef struct
{
    char logs[SYSTEM_LOG_MAX_LINES][SYSTEM_LOG_MAX_LENGTH];
    int log_count;
    bool dirty; // 새 로그가 추가되었는지 여부
} SystemLogDisplay;

// 초기화
void system_log_display_init(SystemLogDisplay *display);

// 로그 추가 (자동으로 스크롤)
void system_log_display_add(SystemLogDisplay *display, const char *message);

// 그리기 (항상 그림)
void system_log_display_draw(WINDOW *win, const SystemLogDisplay *display);

// 선택적 렌더링 (dirty flag 기반)
// 반환값: 렌더링이 발생했으면 true
bool system_log_display_render(WINDOW *win,
                               SystemLogDisplay *display,
                               UIRenderFlags *flags,
                               bool force_render);

// 로그 초기화 (모든 로그 삭제)
void system_log_display_clear(SystemLogDisplay *display);

#endif // SYSTEM_LOG_DISPLAY_H
