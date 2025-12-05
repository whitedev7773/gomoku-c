#include "log_ui.h"
#include <string.h>

void log_init(LogUI *ui)
{
    if (!ui)
        return;

    memset(ui, 0, sizeof(LogUI));
    ui->message_count = 0;
    ui->start_index = 0;
    ui->dirty = true; // 초기에는 전체 렌더링 필요
    ui->prev_message_count = 0;
}

void log_add_msg(LogUI *ui, const char *message)
{
    if (!ui || !message)
        return;

    int index = ui->start_index;

    strncpy(ui->messages[index].message, message, MAX_LOG_MESSAGE_LENGTH - 1);
    ui->messages[index].message[MAX_LOG_MESSAGE_LENGTH - 1] = '\0';
    ui->messages[index].timestamp = time(NULL);

    ui->start_index = (ui->start_index + 1) % MAX_LOG_MESSAGES;

    if (ui->message_count < MAX_LOG_MESSAGES)
    {
        ui->message_count++;
    }

    ui->dirty = true; // 메시지 추가됨
}

void log_render(WINDOW *win, const LogUI *ui, int start_y, int start_x)
{
    if (!win || !ui)
        return;

    int display_count = (ui->message_count < MAX_LOG_MESSAGES) ? ui->message_count : MAX_LOG_MESSAGES;

    for (int i = 0; i < display_count; i++)
    {
        int index = (ui->start_index - display_count + i + MAX_LOG_MESSAGES) % MAX_LOG_MESSAGES;
        mvwprintw(win, start_y + i, start_x, " [SYSTEM] %-40s ",
                  ui->messages[index].message);
    }
}

// ============================================
// 선택적 렌더링 함수 (Dirty Flag 기반)
// ============================================

void log_render_sel(WINDOW *win, LogUI *ui,
                             UIRenderFlags *flags, bool first_render,
                             int start_y, int start_x)
{
    if (!win || !ui)
        return;

    // 첫 렌더링이거나 로그가 변경됨
    if (first_render || ui_render_flags_is_set(flags, RENDER_LOG) || ui->dirty)
    {
        // 로그 영역만 클리어하고 다시 그리기
        for (int i = 0; i < MAX_LOG_MESSAGES; i++)
        {
            wmove(win, start_y + i, start_x);
            wclrtoeol(win);
        }

        log_render(win, ui, start_y, start_x);
        wrefresh(win);
        ui->dirty = false;
        ui->prev_message_count = ui->message_count;
        ui_render_flags_clear(flags, RENDER_LOG);
    }
}

bool log_is_dirty(const LogUI *ui)
{
    return ui->dirty;
}

void log_clear_dirty(LogUI *ui)
{
    ui->dirty = false;
}
