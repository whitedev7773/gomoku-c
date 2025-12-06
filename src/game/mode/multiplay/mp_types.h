#ifndef MP_TYPES_H
#define MP_TYPES_H

#include "../../core/board.h"
#include "../../core/game_logic.h"
#include "../../core/turn_manager.h"
#include "../../feature/game_logger.h"
#include "../../feature/command.h"
#include "../../../ui/core/ui_manager.h"
#include "../../../ui/game/board/board_ui.h"
#include "../../../ui/game/game_info_ui.h"
#include "../../../ui/game/log/log_ui.h"
#include "../../../ui/game/chat/chat_ui.h"
#include "../../../ui/menu/modal_ui.h"
#include "../../../ui/core/input_handler.h"
#include "../../../network/core/network.h"
#include "../../../network/core/protocol.h"

// ============================================================================
// 게임 결과 정의 (관전자 브로드캐스트용)
// ============================================================================
#define RESULT_BLACK_WIN 0
#define RESULT_WHITE_WIN 1
#define RESULT_DRAW 2

#define REASON_FIVE_IN_A_ROW 0
#define REASON_GIVEUP 1
#define REASON_QUIT 2
#define REASON_TIMEOUT 3

// ============================================================================
// 플레이어 정보
// ============================================================================
typedef struct
{
    char name[MAX_PLAYER_NAME];
    Stone color;
} PlayerData;

// ============================================================================
// 멀티플레이 게임 상태
// ============================================================================
typedef struct
{
    Board board;
    BoardCursor my_cursor;
    BoardCursor opponent_cursor;
    TurnManager turn_mgr;
    GameInfoUI info_ui;
    LogUI log_ui;
    ChatUI chat_ui;
    ModalUI modal_ui;
    GameLogger logger;

    PlayerData me;
    PlayerData opponent;

    NetworkManager network;
    bool waiting_for_opponent;
    bool game_over;
    bool quit_requested;
    GameResult result;
    bool swap_used;
    bool first_render;

    // 터미널 크기 경고 상태
    bool terminal_warning_shown;

    // 게임 시간 추적
    time_t game_start_time;
} MultiplayerGame;

// ============================================================================
// 모달 입력 처리 결과
// ============================================================================
typedef enum
{
    MP_MODAL_NONE,     // 처리 없음
    MP_MODAL_CLOSED,   // 모달이 닫힘
    MP_MODAL_GAME_OVER // 게임 종료 (기권 등)
} MpModalResult;

#endif // MP_TYPES_H
