#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../game/core/board.h"

#define MAX_PLAYER_NAME 8
#define PROTOCOL_VERSION 1

// 메시지 타입
typedef enum
{
    MSG_CONNECT = 1,                // 연결 요청
    MSG_CONNECT_ACK = 2,            // 연결 승인
    MSG_PLAYER_INFO = 3,            // 플레이어 정보
    MSG_GAME_START = 4,             // 게임 시작
    MSG_MOVE = 5,                   // 돌 놓기
    MSG_MOVE_ACK = 6,               // 수 승인
    MSG_CURSOR_UPDATE = 7,          // 커서 위치 업데이트
    MSG_CHAT = 8,                   // 채팅 메시지
    MSG_PING = 9,                   // PING 요청
    MSG_PONG = 10,                  // PONG 응답
    MSG_DISCONNECT = 11,            // 연결 종료
    MSG_COMMAND = 12,               // 명령어
    MSG_COMMAND_RESPONSE = 13,      // 명령어 응답
    MSG_SPECTATOR_CONNECT = 14,     // 관전자 연결 요청
    MSG_SPECTATOR_CONNECT_ACK = 15, // 관전자 연결 승인/거부
    MSG_SPECTATOR_JOIN = 16,        // 관전자 입장 알림
    MSG_SPECTATOR_LEAVE = 17,       // 관전자 퇴장 알림
    MSG_GAME_STATE = 18,            // 전체 게임 상태
    MSG_GAME_RESULT = 19,           // 게임 결과 (승/패/기권/퇴장)
    MSG_ERROR = 99                  // 에러
} MessageType;

// 에러 코드
typedef enum
{
    ERR_NONE = 0,
    ERR_INVALID_MOVE = 1,
    ERR_NOT_YOUR_TURN = 2,
    ERR_GAME_FULL = 3,
    ERR_VERSION_MISMATCH = 4,
    ERR_NO_GAME = 5,        // 진행 중인 게임 없음
    ERR_SPECTATOR_FULL = 6, // 관전자 수 초과 (최대 3명)
    ERR_UNKNOWN = 99
} ErrorCode;

// 메시지 헤더 (모든 메시지 공통)
typedef struct __attribute__((packed))
{
    uint8_t version;   // 프로토콜 버전
    uint8_t type;      // MessageType
    uint16_t length;   // 페이로드 길이
    uint32_t sequence; // 시퀀스 번호
} MessageHeader;

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

// 게임 시작
typedef struct __attribute__((packed))
{
    uint8_t your_turn; // 1 if your turn, 0 otherwise
} GameStartMessage;

// 돌 놓기
typedef struct __attribute__((packed))
{
    uint8_t row;
    uint8_t col;
    uint8_t stone; // Stone
} MoveMessage;

// 수 승인
typedef struct __attribute__((packed))
{
    uint8_t accepted;   // 1 if accepted, 0 if rejected
    uint8_t error_code; // ErrorCode (if rejected)
} MoveAckMessage;

// 커서 업데이트
typedef struct __attribute__((packed))
{
    uint8_t row;
    uint8_t col;
} CursorUpdateMessage;

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

// 전체 게임 상태 (관전자 접속 시)
typedef struct __attribute__((packed))
{
    char player1_name[MAX_PLAYER_NAME];
    char player2_name[MAX_PLAYER_NAME];
    uint8_t current_turn;                         // Stone (BLACK or WHITE)
    uint8_t move_count;                           // 총 수 개수
    uint8_t board_state[BOARD_SIZE * BOARD_SIZE]; // 보드 상태 (EMPTY=0, BLACK=1, WHITE=2)
} GameStateMessage;

// 게임 결과 메시지 (관전자에게 전송)
typedef struct __attribute__((packed))
{
    uint8_t result_type; // 0=BLACK_WIN, 1=WHITE_WIN, 2=DRAW
    uint8_t reason;      // 0=FIVE_IN_A_ROW, 1=GIVEUP, 2=QUIT, 3=TIMEOUT
    char winner_name[MAX_PLAYER_NAME];
    char message[64]; // 결과 메시지
} GameResultMessage;

// 전체 메시지 (헤더 + 페이로드)
typedef struct
{
    MessageHeader header;
    union
    {
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
        SpectatorConnectMessage spectator_connect;
        SpectatorConnectAckMessage spectator_connect_ack;
        SpectatorJoinLeaveMessage spectator_join_leave;
        GameStateMessage game_state;
        GameResultMessage game_result;
    } payload;
} Message;

// 프로토콜 함수
void protocol_init_message(Message *msg, MessageType type, uint32_t sequence);
int protocol_serialize(const Message *msg, uint8_t *buffer, size_t buffer_size);
int protocol_deserialize(Message *msg, const uint8_t *buffer, size_t buffer_size);
size_t protocol_get_message_size(MessageType type);

#endif // PROTOCOL_H
