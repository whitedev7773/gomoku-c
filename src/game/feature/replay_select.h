#ifndef REPLAY_SELECT_H
#define REPLAY_SELECT_H

#include "game_logger.h"
#include <stdbool.h>
#include <time.h>

#define MAX_LOG_FILES 100

// 로그 파일 정보
typedef struct
{
    char filename[LOG_FILENAME_SIZE];
    char display_name[LOG_FILENAME_SIZE + 16]; // 표시용 이름
    time_t modified_time;
} LogFileInfo;

// 로그 파일 목록
typedef struct
{
    LogFileInfo files[MAX_LOG_FILES];
    int file_count;
} LogFileList;

// 로그 파일 목록 가져오기
bool replay_get_log_files(LogFileList *list);

// 파일 선택 UI 실행 (선택된 파일명 반환, NULL이면 취소)
// 내부적으로 리플레이 뷰어를 실행하고 돌아옴
int replay_run_with_selection(void);

#endif // REPLAY_SELECT_H
