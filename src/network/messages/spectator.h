#ifndef MSG_SPECTATOR_H
#define MSG_SPECTATOR_H

#include "message_types.h"

// 관전자 연결 요청
typedef struct __attribute__((packed))
{
    char spectator_name[MAX_PLAYER_NAME];
} SpectatorConnectMessage;

// 관전자 연결 승인/거부
typedef struct __attribute__((packed))
{
    uint8_t accepted;        // 1 if accepted, 0 if rejected
    uint8_t error_code;      // ErrorCode (if rejected)
    uint8_t spectator_count; // 현재 관전자 수
    uint8_t max_spectators;  // 최대 관전자 수 (3)
} SpectatorConnectAckMessage;

// 관전자 입장/퇴장 알림
typedef struct __attribute__((packed))
{
    char spectator_name[MAX_PLAYER_NAME];
    uint8_t spectator_count; // 현재 관전자 수
} SpectatorJoinLeaveMessage;

#endif // MSG_SPECTATOR_H
