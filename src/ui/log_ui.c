#include "log_ui.h"
#include <string.h>

void log_ui_init(LogUI *ui) {
    if (!ui) return;

    memset(ui, 0, sizeof(LogUI));
    ui->message_count = 0;
    ui->start_index = 0;
}

void log_ui_add_message(LogUI *ui, const char *message) {
    if (!ui || !message) return;

    int index = ui->start_index;

    strncpy(ui->messages[index].message, message, MAX_LOG_MESSAGE_LENGTH - 1);
    ui->messages[index].message[MAX_LOG_MESSAGE_LENGTH - 1] = '\0';
    ui->messages[index].timestamp = time(NULL);

    ui->start_index = (ui->start_index + 1) % MAX_LOG_MESSAGES;

    if (ui->message_count < MAX_LOG_MESSAGES) {
        ui->message_count++;
    }
}

void log_ui_render(WINDOW *win, const LogUI *ui, int start_y, int start_x) {
    if (!win || !ui) return;

    int display_count = (ui->message_count < MAX_LOG_MESSAGES) ?
                        ui->message_count : MAX_LOG_MESSAGES;

    for (int i = 0; i < display_count; i++) {
        int index = (ui->start_index - display_count + i + MAX_LOG_MESSAGES) % MAX_LOG_MESSAGES;
        mvwprintw(win, start_y + i, start_x, " [SYSTEM] %-40s ",
                  ui->messages[index].message);
    }
}
