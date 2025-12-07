#ifndef ANTI_DEBUG_H
#define ANTI_DEBUG_H

#include <stdbool.h>
#include <stdint.h>

/**
 * 안티-디버깅 감지 타입
 */
typedef enum
{
    DEBUG_NONE = 0,        // 디버거 미감지
    DEBUG_PTRACE_DETECTED, // ptrace 감지
    DEBUG_PROC_STATUS,     // /proc/self/status에서 TracerPid 감지
    DEBUG_TIMING_ATTACK,   // 비정상적 실행 시간 (타이밍 공격)
    DEBUG_BREAKPOINT,      // 브레이크포인트 감지
    DEBUG_PROCESS_NAME     // 디버거 프로세스 감지 (gdb, lldb 등)
} DebugDetectionType;

/**
 * 안티-디버깅 상태
 */
typedef struct
{
    bool ptrace_check_enabled;
    bool proc_status_check_enabled;
    bool timing_check_enabled;
    bool process_check_enabled;

    uint64_t last_check_time;      // 마지막 체크 시간 (나노초)
    uint64_t normal_execution_time; // 정상 실행 시간 기준값 (나노초)

    int check_count;               // 총 체크 횟수
    int detection_count;           // 감지 횟수
} AntiDebugState;

/**
 * 안티-디버깅 초기화
 */
void anti_debug_init(AntiDebugState *state);

/**
 * ptrace를 사용한 디버거 감지
 * @return 디버거 감지 시 true
 */
bool anti_debug_check_ptrace(void);

/**
 * /proc/self/status에서 TracerPid 확인
 * @return 디버거 감지 시 true
 */
bool anti_debug_check_proc_status(void);

/**
 * 디버거 프로세스 이름 감지 (gdb, lldb, strace 등)
 * @return 디버거 프로세스 감지 시 true
 */
bool anti_debug_check_debugger_process(void);

/**
 * 타이밍 공격 감지 (비정상적으로 느린 실행)
 * @param state 안티-디버깅 상태
 * @param execution_time_ns 측정된 실행 시간 (나노초)
 * @return 비정상 실행 시간 감지 시 true
 */
bool anti_debug_check_timing(AntiDebugState *state, uint64_t execution_time_ns);

/**
 * 종합 디버거 체크 (모든 방법 사용)
 * @param state 안티-디버깅 상태
 * @param out_type 감지된 디버깅 타입 (출력)
 * @return 디버거 감지 시 true
 */
bool anti_debug_comprehensive_check(AntiDebugState *state, DebugDetectionType *out_type);

/**
 * 현재 시간 가져오기 (나노초)
 */
uint64_t anti_debug_get_time_ns(void);

/**
 * 디버깅 감지 타입을 문자열로 변환
 */
const char *anti_debug_type_to_string(DebugDetectionType type);

#endif // ANTI_DEBUG_H
