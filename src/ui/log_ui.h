#ifndef LOG_UI_H
#define LOG_UI_H

#include <ncurses.h>
#include <time.h>

#define MAX_LOG_MESSAGES 3
#define MAX_LOG_MESSAGE_LENGTH 64

typedef struct {
    char message[MAX_LOG_MESSAGE_LENGTH];
    time_t timestamp;
} LogMessage;

typedef struct {
    LogMessage messages[MAX_LOG_MESSAGES];
    int message_count;
    int start_index;
} LogUI;

void log_ui_init(LogUI *ui);

void log_ui_add_message(LogUI *ui, const char *message);

void log_ui_render(WINDOW *win, const LogUI *ui, int start_y, int start_x);

#endif // LOG_UI_H
