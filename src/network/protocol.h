#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../game/board.h"

#define MAX_PLAYER_NAME 8
#define PROTOCOL_VERSION 1

// 메시지 타입
typedef enum {
    MSG_CONNECT = 1,        // 연결 요청
    MSG_CONNECT_ACK = 2,    // 연결 승인
    MSG_PLAYER_INFO = 3,    // 플레이어 정보
    MSG_GAME_START = 4,     // 게임 시작
    MSG_MOVE = 5,           // 돌 놓기
    MSG_MOVE_ACK = 6,       // 수 승인
    MSG_CURSOR_UPDATE = 7,  // 커서 위치 업데이트
    MSG_CHAT = 8,           // 채팅 메시지
    MSG_PING = 9,           // PING 요청
    MSG_PONG = 10,          // PONG 응답
    MSG_DISCONNECT = 11,    // 연결 종료
    MSG_COMMAND = 12,       // 명령어
    MSG_COMMAND_RESPONSE = 13,  // 명령어 응답
    MSG_ERROR = 99          // 에러
} MessageType;

// 에러 코드
typedef enum {
    ERR_NONE = 0,
    ERR_INVALID_MOVE = 1,
    ERR_NOT_YOUR_TURN = 2,
    ERR_GAME_FULL = 3,
    ERR_VERSION_MISMATCH = 4,
    ERR_UNKNOWN = 99
} ErrorCode;

// 메시지 헤더 (모든 메시지 공통)
typedef struct __attribute__((packed)) {
    uint8_t version;        // 프로토콜 버전
    uint8_t type;           // MessageType
    uint16_t length;        // 페이로드 길이
    uint32_t sequence;      // 시퀀스 번호
} MessageHeader;

// 연결 요청
typedef struct __attribute__((packed)) {
    char player_name[MAX_PLAYER_NAME];
} ConnectMessage;

// 연결 승인
typedef struct __attribute__((packed)) {
    uint8_t your_color;     // Stone (BLACK or WHITE)
    char opponent_name[MAX_PLAYER_NAME];
} ConnectAckMessage;

// 플레이어 정보
typedef struct __attribute__((packed)) {
    char name[MAX_PLAYER_NAME];
    uint8_t color;          // Stone
} PlayerInfoMessage;

// 게임 시작
typedef struct __attribute__((packed)) {
    uint8_t your_turn;      // 1 if your turn, 0 otherwise
} GameStartMessage;

// 돌 놓기
typedef struct __attribute__((packed)) {
    uint8_t row;
    uint8_t col;
    uint8_t stone;          // Stone
} MoveMessage;

// 수 승인
typedef struct __attribute__((packed)) {
    uint8_t accepted;       // 1 if accepted, 0 if rejected
    uint8_t error_code;     // ErrorCode (if rejected)
} MoveAckMessage;

// 커서 업데이트
typedef struct __attribute__((packed)) {
    uint8_t row;
    uint8_t col;
} CursorUpdateMessage;

// 채팅 메시지
typedef struct __attribute__((packed)) {
    char message[64];
} ChatMessage;

// 명령어 메시지
typedef struct __attribute__((packed)) {
    uint8_t command_type;   // CommandType from command.h
    char argument[32];      // 명령어 인자 (선택적)
} CommandMessage;

// 명령어 응답
typedef struct __attribute__((packed)) {
    uint8_t command_type;
    uint8_t accepted;       // 1 if accepted, 0 if rejected
    char message[64];       // 응답 메시지
} CommandResponseMessage;

// PING/PONG
typedef struct __attribute__((packed)) {
    uint64_t timestamp;     // milliseconds since epoch
} PingPongMessage;

// 에러 메시지
typedef struct __attribute__((packed)) {
    uint8_t error_code;     // ErrorCode
    char description[64];
} ErrorMessage;

// 전체 메시지 (헤더 + 페이로드)
typedef struct {
    MessageHeader header;
    union {
        ConnectMessage connect;
        ConnectAckMessage connect_ack;
        PlayerInfoMessage player_info;
        GameStartMessage game_start;
        MoveMessage move;
        MoveAckMessage move_ack;
        CursorUpdateMessage cursor;
        ChatMessage chat;
        CommandMessage command;
        CommandResponseMessage command_response;
        PingPongMessage ping_pong;
        ErrorMessage error;
    } payload;
} Message;

// 프로토콜 함수
void protocol_init_message(Message *msg, MessageType type, uint32_t sequence);
int protocol_serialize(const Message *msg, uint8_t *buffer, size_t buffer_size);
int protocol_deserialize(Message *msg, const uint8_t *buffer, size_t buffer_size);
size_t protocol_get_message_size(MessageType type);

#endif // PROTOCOL_H
