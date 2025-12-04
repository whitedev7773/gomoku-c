#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h>

// 명령어 타입
typedef enum
{
    CMD_NONE,
    CMD_QUIT,   // 게임 퇴장
    CMD_UNDO,   // 무르기 요청
    CMD_GIVEUP, // 기권
    CMD_SWAP,   // 교환 규칙
    CMD_HELP,   // 도움말
    CMD_INVALID // 잘못된 명령어
} CommandType;

// 명령어 결과
typedef struct
{
    CommandType type;
    bool valid;
    char error_message[128];
} CommandResult;

/**
 * 메시지가 명령어인지 확인
 */
bool command_is_command(const char *message);

/**
 * 명령어 파싱
 */
CommandResult command_parse(const char *message);

/**
 * 명령어 타입을 문자열로 변환
 */
const char *command_type_to_string(CommandType type);

#endif // COMMAND_H
