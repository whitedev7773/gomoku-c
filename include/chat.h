#ifndef CHAT_H
#define CHAT_H

#include <stddef.h>

#define CHAT_MAX_MESSAGES 128
#define CHAT_MESSAGE_LEN 256
#define CHAT_SENDER_LEN 32

typedef struct {
    char sender[CHAT_SENDER_LEN];
    char text[CHAT_MESSAGE_LEN];
} chat_message_t;

typedef struct {
    chat_message_t messages[CHAT_MAX_MESSAGES];
    size_t count;
} chat_log_t;

void chat_log_init(chat_log_t *log);
void chat_log_add(chat_log_t *log, const char *sender, const char *text);
const chat_message_t *chat_log_latest(const chat_log_t *log, size_t index_from_end);

#endif // CHAT_H
