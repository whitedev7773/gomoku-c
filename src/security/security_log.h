#ifndef SECURITY_LOG_H
#define SECURITY_LOG_H

#include <stdbool.h>
#include <time.h>

#define SECURITY_LOG_FILE "security.log"
#define MAX_LOG_MESSAGE_LENGTH 512

/**
 * 보안 로그 레벨
 */
typedef enum
{
    SEC_LOG_INFO,    // 정보
    SEC_LOG_WARNING, // 경고
    SEC_LOG_ALERT,   // 알림 (의심스러운 활동)
    SEC_LOG_CRITICAL // 심각 (무결성 위반 등)
} SecurityLogLevel;

/**
 * 보안 이벤트 타입
 */
typedef enum
{
    SEC_EVENT_STARTUP,             // 프로그램 시작
    SEC_EVENT_INTEGRITY_VIOLATION, // 무결성 위반
    SEC_EVENT_DEBUG_DETECTED,      // 디버거 감지
    SEC_EVENT_MEMORY_VIOLATION,    // 메모리 변조 감지
    SEC_EVENT_SUSPICIOUS_ACTIVITY, // 의심스러운 활동
    SEC_EVENT_SHUTDOWN             // 프로그램 종료
} SecurityEventType;

/**
 * 보안 로그 초기화
 * @return 성공 시 true
 */
bool security_log_init(void);

/**
 * 보안 로그 기록
 * @param level 로그 레벨
 * @param event_type 이벤트 타입
 * @param message 메시지
 * @param ... 추가 인자 (printf 형식)
 */
void security_log_write(SecurityLogLevel level, SecurityEventType event_type, const char *message, ...);

/**
 * 보안 이벤트 기록 (간편 함수)
 */
void security_log_info(const char *message, ...);
void security_log_warning(const char *message, ...);
void security_log_alert(const char *message, ...);
void security_log_critical(const char *message, ...);

/**
 * 무결성 위반 기록
 * @param details 위반 상세 정보
 */
void security_log_integrity_violation(const char *details);

/**
 * 디버거 감지 기록
 * @param detection_method 감지 방법
 */
void security_log_debug_detected(const char *detection_method);

/**
 * 메모리 변조 감지 기록
 * @param location 변조된 위치
 */
void security_log_memory_violation(const char *location);

/**
 * 보안 로그 파일 경로 가져오기
 */
const char *security_log_get_filepath(void);

/**
 * 로그 레벨을 문자열로 변환
 */
const char *security_log_level_to_string(SecurityLogLevel level);

/**
 * 이벤트 타입을 문자열로 변환
 */
const char *security_log_event_to_string(SecurityEventType event);

#endif // SECURITY_LOG_H
