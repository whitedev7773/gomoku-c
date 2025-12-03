#ifndef REPLAY_H
#define REPLAY_H

#include "game_logger.h"
#include "board.h"
#include <stdbool.h>

#define MAX_LOG_FILES 100
#define REPLAY_SPEED_SLOW 2000000    // 2초 (microseconds)
#define REPLAY_SPEED_NORMAL 1000000  // 1초
#define REPLAY_SPEED_FAST 500000     // 0.5초

// 로그 파일 정보
typedef struct {
    char filename[LOG_FILENAME_SIZE];
    char display_name[LOG_FILENAME_SIZE + 16];  // 표시용 이름
    time_t modified_time;
} LogFileInfo;

// 로그 파일 목록
typedef struct {
    LogFileInfo files[MAX_LOG_FILES];
    int file_count;
} LogFileList;

// 리플레이 상태
typedef struct {
    GameLogger logger;
    int current_move;
    int total_moves;
    bool playing;
    bool paused;
    int speed;  // microseconds delay between moves
} ReplayState;

// 로그 파일 목록 가져오기
bool replay_get_log_files(LogFileList *list);

// 리플레이 초기화
bool replay_init(ReplayState *replay, const char *log_filename);

// 리플레이 정리
void replay_cleanup(ReplayState *replay);

// 다음 수 재생
bool replay_next_move(ReplayState *replay, Board *board);

// 이전 수로 되돌리기
bool replay_prev_move(ReplayState *replay, Board *board);

// 리플레이 실행 (UI 포함)
int replay_run(const char *log_filename);

// 파일 선택 후 리플레이 실행
int replay_run_with_selection(void);

#endif // REPLAY_H
