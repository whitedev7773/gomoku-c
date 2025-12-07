#include "security_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static FILE *g_log_file = NULL;

bool security_log_init(void)
{
    g_log_file = fopen(SECURITY_LOG_FILE, "a");
    if (!g_log_file)
    {
        return false;
    }

    // 시작 로그 기록
    security_log_write(SEC_LOG_INFO, SEC_EVENT_STARTUP, "Security logging initialized");
    return true;
}

void security_log_write(SecurityLogLevel level, SecurityEventType event_type, const char *message, ...)
{
    if (!g_log_file)
    {
        return;
    }

    // 현재 시간 가져오기
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buffer[64];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

    // 가변 인자 처리
    char formatted_message[MAX_LOG_MESSAGE_LENGTH];
    va_list args;
    va_start(args, message);
    vsnprintf(formatted_message, sizeof(formatted_message), message, args);
    va_end(args);

    // 로그 기록
    fprintf(g_log_file, "[%s] [%s] [%s] %s\n",
            time_buffer,
            security_log_level_to_string(level),
            security_log_event_to_string(event_type),
            formatted_message);

    // 즉시 디스크에 기록 (보안 로그는 중요하므로)
    fflush(g_log_file);
}

void security_log_info(const char *message, ...)
{
    if (!g_log_file)
    {
        return;
    }

    char formatted_message[MAX_LOG_MESSAGE_LENGTH];
    va_list args;
    va_start(args, message);
    vsnprintf(formatted_message, sizeof(formatted_message), message, args);
    va_end(args);

    security_log_write(SEC_LOG_INFO, SEC_EVENT_SUSPICIOUS_ACTIVITY, "%s", formatted_message);
}

void security_log_warning(const char *message, ...)
{
    if (!g_log_file)
    {
        return;
    }

    char formatted_message[MAX_LOG_MESSAGE_LENGTH];
    va_list args;
    va_start(args, message);
    vsnprintf(formatted_message, sizeof(formatted_message), message, args);
    va_end(args);

    security_log_write(SEC_LOG_WARNING, SEC_EVENT_SUSPICIOUS_ACTIVITY, "%s", formatted_message);
}

void security_log_alert(const char *message, ...)
{
    if (!g_log_file)
    {
        return;
    }

    char formatted_message[MAX_LOG_MESSAGE_LENGTH];
    va_list args;
    va_start(args, message);
    vsnprintf(formatted_message, sizeof(formatted_message), message, args);
    va_end(args);

    security_log_write(SEC_LOG_ALERT, SEC_EVENT_SUSPICIOUS_ACTIVITY, "%s", formatted_message);
}

void security_log_critical(const char *message, ...)
{
    if (!g_log_file)
    {
        return;
    }

    char formatted_message[MAX_LOG_MESSAGE_LENGTH];
    va_list args;
    va_start(args, message);
    vsnprintf(formatted_message, sizeof(formatted_message), message, args);
    va_end(args);

    security_log_write(SEC_LOG_CRITICAL, SEC_EVENT_SUSPICIOUS_ACTIVITY, "%s", formatted_message);
}

void security_log_integrity_violation(const char *details)
{
    security_log_write(SEC_LOG_CRITICAL, SEC_EVENT_INTEGRITY_VIOLATION,
                       "Integrity violation detected: %s", details);
}

void security_log_debug_detected(const char *detection_method)
{
    security_log_write(SEC_LOG_ALERT, SEC_EVENT_DEBUG_DETECTED,
                       "Debugger detected via: %s", detection_method);
}

void security_log_memory_violation(const char *location)
{
    security_log_write(SEC_LOG_CRITICAL, SEC_EVENT_MEMORY_VIOLATION,
                       "Memory tampering detected at: %s", location);
}

const char *security_log_get_filepath(void)
{
    return SECURITY_LOG_FILE;
}

const char *security_log_level_to_string(SecurityLogLevel level)
{
    switch (level)
    {
    case SEC_LOG_INFO:
        return "INFO";
    case SEC_LOG_WARNING:
        return "WARNING";
    case SEC_LOG_ALERT:
        return "ALERT";
    case SEC_LOG_CRITICAL:
        return "CRITICAL";
    default:
        return "UNKNOWN";
    }
}

const char *security_log_event_to_string(SecurityEventType event)
{
    switch (event)
    {
    case SEC_EVENT_STARTUP:
        return "STARTUP";
    case SEC_EVENT_INTEGRITY_VIOLATION:
        return "INTEGRITY_VIOLATION";
    case SEC_EVENT_DEBUG_DETECTED:
        return "DEBUG_DETECTED";
    case SEC_EVENT_MEMORY_VIOLATION:
        return "MEMORY_VIOLATION";
    case SEC_EVENT_SUSPICIOUS_ACTIVITY:
        return "SUSPICIOUS_ACTIVITY";
    case SEC_EVENT_SHUTDOWN:
        return "SHUTDOWN";
    default:
        return "UNKNOWN";
    }
}
