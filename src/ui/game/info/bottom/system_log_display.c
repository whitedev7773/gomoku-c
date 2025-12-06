#include "system_log_display.h"
#include <stdio.h>
#include <string.h>

void system_log_display_init(SystemLogDisplay *display)
{
    if (!display)
        return;

    for (int i = 0; i < SYSTEM_LOG_MAX_LINES; i++)
    {
        memset(display->logs[i], 0, SYSTEM_LOG_MAX_LENGTH);
    }
    display->log_count = 0;
    display->dirty = false;
}

void system_log_display_add(SystemLogDisplay *display, const char *message)
{
    if (!display || !message)
        return;

    // 로그가 가득 찼으면 스크롤 (첫 번째 로그 삭제)
    if (display->log_count >= SYSTEM_LOG_MAX_LINES)
    {
        for (int i = 0; i < SYSTEM_LOG_MAX_LINES - 1; i++)
        {
            strncpy(display->logs[i], display->logs[i + 1], SYSTEM_LOG_MAX_LENGTH - 1);
            display->logs[i][SYSTEM_LOG_MAX_LENGTH - 1] = '\0';
        }
        display->log_count = SYSTEM_LOG_MAX_LINES - 1;
    }

    // 새 로그 추가
    strncpy(display->logs[display->log_count], message, SYSTEM_LOG_MAX_LENGTH - 1);
    display->logs[display->log_count][SYSTEM_LOG_MAX_LENGTH - 1] = '\0';
    display->log_count++;
    display->dirty = true;
}

void system_log_display_draw(WINDOW *win, const SystemLogDisplay *display)
{
    if (!win || !display)
        return;

    // SYSTEM LOG 영역: (32,26) 시작, 68x5
    // 내부 좌표: (33,27) ~ (98,31)
    // 실제 로그 표시 영역: 3줄

    for (int i = 0; i < SYSTEM_LOG_MAX_LINES; i++)
    {
        // 각 줄 초기화 후 로그 출력
        mvprintw(27 + i, 33, "%-64s", "");
        if (i < display->log_count)
        {
            mvprintw(27 + i, 33, "%s", display->logs[i]);
        }
    }
    refresh();
}

bool system_log_display_render(WINDOW *win,
                               SystemLogDisplay *display,
                               UIRenderFlags *flags,
                               bool force_render)
{
    if (!win || !display || !flags)
        return false;

    bool rendered = false;

    if (force_render || ui_render_flags_is_set(flags, RENDER_LOG) || display->dirty)
    {
        system_log_display_draw(win, display);
        display->dirty = false;
        rendered = true;

        ui_render_flags_clear(flags, RENDER_LOG);
    }

    return rendered;
}

void system_log_display_clear(SystemLogDisplay *display)
{
    if (!display)
        return;

    for (int i = 0; i < SYSTEM_LOG_MAX_LINES; i++)
    {
        memset(display->logs[i], 0, SYSTEM_LOG_MAX_LENGTH);
    }
    display->log_count = 0;
    display->dirty = true;
}
