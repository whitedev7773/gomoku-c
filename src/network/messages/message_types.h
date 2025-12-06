#ifndef MESSAGE_TYPES_H
#define MESSAGE_TYPES_H

#include <stdint.h>

// 프로토콜 상수
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

#endif // MESSAGE_TYPES_H
