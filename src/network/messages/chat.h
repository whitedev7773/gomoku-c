#ifndef MSG_CHAT_H
#define MSG_CHAT_H

#include "message_types.h"

// 채팅 메시지
typedef struct __attribute__((packed))
{
    char message[64];
} ChatMessage;

// 명령어 메시지
typedef struct __attribute__((packed))
{
    uint8_t command_type; // CommandType from command.h
    char argument[32];    // 명령어 인자 (선택적)
} CommandMessage;

// 명령어 응답
typedef struct __attribute__((packed))
{
    uint8_t command_type;
    uint8_t accepted; // 1 if accepted, 0 if rejected
    char message[64]; // 응답 메시지
} CommandResponseMessage;

#endif // MSG_CHAT_H
