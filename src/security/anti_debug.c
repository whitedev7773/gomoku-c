#include "anti_debug.h"
#include "security_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <time.h>
#include <dirent.h>

#define TIMING_THRESHOLD_MULTIPLIER 10 // 정상 실행 시간의 10배 이상이면 의심

void anti_debug_init(AntiDebugState *state)
{
    if (!state)
    {
        return;
    }

    memset(state, 0, sizeof(AntiDebugState));

    state->ptrace_check_enabled = true;
    state->proc_status_check_enabled = true;
    state->timing_check_enabled = true;
    state->process_check_enabled = true;

    state->normal_execution_time = 1000000; // 1ms (기준값, 나노초)
}

bool anti_debug_check_ptrace(void)
{
    // ptrace(PTRACE_TRACEME, 0, NULL, NULL)을 사용하여 디버거 감지
    // 이미 디버거가 attach되어 있으면 ptrace가 실패함
    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1)
    {
        security_log_debug_detected("ptrace(PTRACE_TRACEME)");
        return true; // 디버거 감지됨
    }

    // 자기 자신을 detach
    ptrace(PTRACE_DETACH, 0, NULL, NULL);
    return false;
}

bool anti_debug_check_proc_status(void)
{
    FILE *status_file = fopen("/proc/self/status", "r");
    if (!status_file)
    {
        return false;
    }

    char line[256];
    while (fgets(line, sizeof(line), status_file))
    {
        if (strncmp(line, "TracerPid:", 10) == 0)
        {
            int tracer_pid = 0;
            sscanf(line + 10, "%d", &tracer_pid);

            fclose(status_file);

            if (tracer_pid != 0)
            {
                security_log_debug_detected("/proc/self/status (TracerPid)");
                return true; // 디버거 감지됨
            }
            return false;
        }
    }

    fclose(status_file);
    return false;
}

bool anti_debug_check_debugger_process(void)
{
    DIR *proc_dir = opendir("/proc");
    if (!proc_dir)
    {
        return false;
    }

    const char *debugger_names[] = {"gdb", "lldb", "strace", "ltrace", "radare2", "r2", "ida", "ida64"};
    const int debugger_count = sizeof(debugger_names) / sizeof(debugger_names[0]);

    struct dirent *entry;
    while ((entry = readdir(proc_dir)) != NULL)
    {
        // 숫자로 시작하는 디렉토리만 (프로세스 ID)
        if (entry->d_type == DT_DIR && entry->d_name[0] >= '0' && entry->d_name[0] <= '9')
        {
            char cmdline_path[512];
            snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%s/cmdline", entry->d_name);

            FILE *cmdline_file = fopen(cmdline_path, "r");
            if (cmdline_file)
            {
                char cmdline[256];
                if (fgets(cmdline, sizeof(cmdline), cmdline_file))
                {
                    // 디버거 프로세스 이름 검사
                    for (int i = 0; i < debugger_count; i++)
                    {
                        if (strstr(cmdline, debugger_names[i]))
                        {
                            fclose(cmdline_file);
                            closedir(proc_dir);
                            security_log_debug_detected("Debugger process found");
                            return true;
                        }
                    }
                }
                fclose(cmdline_file);
            }
        }
    }

    closedir(proc_dir);
    return false;
}

bool anti_debug_check_timing(AntiDebugState *state, uint64_t execution_time_ns)
{
    if (!state || !state->timing_check_enabled)
    {
        return false;
    }

    // 정상 실행 시간의 임계값 초과 시 의심
    uint64_t threshold = state->normal_execution_time * TIMING_THRESHOLD_MULTIPLIER;

    if (execution_time_ns > threshold)
    {
        security_log_debug_detected("Timing attack (abnormally slow execution)");
        return true;
    }

    return false;
}

bool anti_debug_comprehensive_check(AntiDebugState *state, DebugDetectionType *out_type)
{
    if (!state)
    {
        return false;
    }

    state->check_count++;

    // ptrace 체크
    if (state->ptrace_check_enabled && anti_debug_check_ptrace())
    {
        if (out_type)
        {
            *out_type = DEBUG_PTRACE_DETECTED;
        }
        state->detection_count++;
        return true;
    }

    // /proc/self/status 체크
    if (state->proc_status_check_enabled && anti_debug_check_proc_status())
    {
        if (out_type)
        {
            *out_type = DEBUG_PROC_STATUS;
        }
        state->detection_count++;
        return true;
    }

    // 디버거 프로세스 체크
    if (state->process_check_enabled && anti_debug_check_debugger_process())
    {
        if (out_type)
        {
            *out_type = DEBUG_PROCESS_NAME;
        }
        state->detection_count++;
        return true;
    }

    if (out_type)
    {
        *out_type = DEBUG_NONE;
    }
    return false;
}

uint64_t anti_debug_get_time_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

const char *anti_debug_type_to_string(DebugDetectionType type)
{
    switch (type)
    {
    case DEBUG_NONE:
        return "None";
    case DEBUG_PTRACE_DETECTED:
        return "ptrace Detected";
    case DEBUG_PROC_STATUS:
        return "/proc/self/status (TracerPid)";
    case DEBUG_TIMING_ATTACK:
        return "Timing Attack";
    case DEBUG_BREAKPOINT:
        return "Breakpoint Detected";
    case DEBUG_PROCESS_NAME:
        return "Debugger Process Detected";
    default:
        return "Unknown";
    }
}
