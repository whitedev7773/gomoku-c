#include "chat.h"

#include <string.h>

void chat_log_init(chat_log_t *log) {
    if (!log) {
        return;
    }
    memset(log, 0, sizeof(*log));
}

void chat_log_add(chat_log_t *log, const char *sender, const char *text) {
    if (!log || !sender || !text) {
        return;
    }
    if (log->count >= CHAT_MAX_MESSAGES) {
        memmove(&log->messages[0], &log->messages[1], sizeof(chat_message_t) * (CHAT_MAX_MESSAGES - 1));
        log->count = CHAT_MAX_MESSAGES - 1;
    }
    chat_message_t *msg = &log->messages[log->count++];
    strncpy(msg->sender, sender, CHAT_SENDER_LEN - 1);
    msg->sender[CHAT_SENDER_LEN - 1] = '\0';
    strncpy(msg->text, text, CHAT_MESSAGE_LEN - 1);
    msg->text[CHAT_MESSAGE_LEN - 1] = '\0';
}

const chat_message_t *chat_log_latest(const chat_log_t *log, size_t index_from_end) {
    if (!log || log->count == 0) {
        return NULL;
    }
    if (index_from_end >= log->count) {
        return NULL;
    }
    size_t idx = log->count - 1 - index_from_end;
    return &log->messages[idx];
}
