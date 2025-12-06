#ifndef MSG_GAME_H
#define MSG_GAME_H

#include "message_types.h"
#include "../../game/core/board.h"

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

#endif // MSG_GAME_H
