#ifndef MSG_CONNECTION_H
#define MSG_CONNECTION_H

#include "message_types.h"
#include <stdbool.h>

// 연결 요청
typedef struct __attribute__((packed))
{
    char player_name[MAX_PLAYER_NAME];
} ConnectMessage;

// 연결 승인
typedef struct __attribute__((packed))
{
    uint8_t your_color; // Stone (BLACK or WHITE)
    uint8_t game_rule;  // GameRule (RULE_STANDARD or RULE_RENJU)
    char opponent_name[MAX_PLAYER_NAME];
} ConnectAckMessage;

// 플레이어 정보
typedef struct __attribute__((packed))
{
    char name[MAX_PLAYER_NAME];
    uint8_t color; // Stone
} PlayerInfoMessage;

// PING/PONG
typedef struct __attribute__((packed))
{
    uint64_t timestamp; // milliseconds since epoch
} PingPongMessage;

// 에러 메시지
typedef struct __attribute__((packed))
{
    uint8_t error_code; // ErrorCode
    char description[64];
} ErrorMessage;

#endif // MSG_CONNECTION_H
