#ifndef LOG_UI_H
#define LOG_UI_H

#include <ncurses.h>
#include <time.h>
#include <stdbool.h>
#include "../../core/ui_manager.h"

#define MAX_LOG_MESSAGES 3
#define MAX_LOG_MESSAGE_LENGTH 64

typedef struct
{
    char message[MAX_LOG_MESSAGE_LENGTH];
    time_t timestamp;
} LogMessage;

typedef struct
{
    LogMessage messages[MAX_LOG_MESSAGES];
    int message_count;
    int start_index;
    bool dirty; // 로그가 변경됨
    int prev_message_count;
} LogUI;

void log_init(LogUI *ui);

void log_add_msg(LogUI *ui, const char *message);

void log_render(WINDOW *win, const LogUI *ui, int start_y, int start_x);

// ============================================
// 선택적 렌더링 함수 (Dirty Flag 기반)
// ============================================

void log_render_sel(WINDOW *win, LogUI *ui,
                             UIRenderFlags *flags, bool first_render,
                             int start_y, int start_x);

bool log_is_dirty(const LogUI *ui);
void log_clear_dirty(LogUI *ui);

#endif // LOG_UI_H
