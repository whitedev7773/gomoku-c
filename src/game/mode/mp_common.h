#ifndef MP_COMMON_H
#define MP_COMMON_H

#include "../core/board.h"
#include "../core/game_logic.h"
#include "../core/turn_manager.h"
#include "../feature/game_logger.h"
#include "../feature/command.h"
#include "../../ui/core/ui_manager.h"
#include "../../ui/game/board/board_ui.h"
#include "../../ui/game/game_info_ui.h"
#include "../../ui/game/log/log_ui.h"
#include "../../ui/game/chat/chat_ui.h"
#include "../../ui/menu/modal_ui.h"
#include "../../ui/core/input_handler.h"
#include "../../network/network.h"
#include "../../network/protocol.h"

// 게임 결과 정의 (관전자 브로드캐스트용)
#define RESULT_BLACK_WIN 0
#define RESULT_WHITE_WIN 1
#define RESULT_DRAW 2

#define REASON_FIVE_IN_A_ROW 0
#define REASON_GIVEUP 1
#define REASON_QUIT 2
#define REASON_TIMEOUT 3

// 플레이어 정보
typedef struct
{
    char name[MAX_PLAYER_NAME];
    Stone color;
} PlayerData;

// 멀티플레이 게임 상태
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
} MultiplayerGame;

/**
 * 모달 입력 처리 결과
 */
typedef enum
{
    MP_MODAL_NONE,     // 처리 없음
    MP_MODAL_CLOSED,   // 모달이 닫힘
    MP_MODAL_GAME_OVER // 게임 종료 (기권 등)
} MpModalResult;

/**
 * 공통 모달 입력 처리 (게임패드 지원)
 * @return MpModalResult
 */
MpModalResult mp_handle_modal_input(MultiplayerGame *game, InputAction action);

/**
 * 채팅 입력 처리 (명령어 포함)
 * @param is_my_turn 내 턴인지 여부 (Undo 가능 여부 결정)
 * @return true if 채팅 모드 종료
 */
bool mp_handle_chat_input(MultiplayerGame *game, int ch, bool is_my_turn);

/**
 * 커서 이동 및 네트워크 전송
 */
void mp_move_cursor_and_send(MultiplayerGame *game, UIRenderFlags *render_flags, int row_delta, int col_delta);

/**
 * 관전자에게 커서 브로드캐스트
 */
void mp_broadcast_cursor_to_spectators(MultiplayerGame *game, int row, int col);

/**
 * 관전자에게 게임 결과 브로드캐스트
 */
void mp_broadcast_game_result_to_spectators(MultiplayerGame *game, uint8_t result_type, uint8_t reason, const char *winner_name, const char *message);

/**
 * 관전자에게 수 브로드캐스트
 */
void mp_broadcast_move_to_spectators(MultiplayerGame *game, const Message *move_msg);

/**
 * 관전자에게 채팅 브로드캐스트
 */
void mp_broadcast_chat_to_spectators(MultiplayerGame *game, const Message *chat_msg);

/**
 * 에러 체크를 포함한 네트워크 메시지 전송
 */
bool mp_send_with_error_check(MultiplayerGame *game, const Message *msg, const char *error_context);

// ============================================================================
// 게임 초기화/정리 함수
// ============================================================================

/**
 * ncurses 및 게임 컴포넌트 초기화
 * @param ui_mgr UI Manager (출력)
 * @param game 게임 상태
 * @param rule 게임 규칙
 * @return 성공 시 true
 */
bool mp_init_game_ui(UIManager *ui_mgr, MultiplayerGame *game, GameRule rule);

/**
 * 게임 정리 (ncurses, 네트워크 등)
 */
void mp_cleanup_game(UIManager *ui_mgr, MultiplayerGame *game, InputHandler *input_handler);

// ============================================================================
// 게임 루프 함수
// ============================================================================

/**
 * 네트워크 메시지 처리
 * @return 메시지가 처리되었으면 true
 */
bool mp_handle_network_messages(UIManager *ui_mgr, MultiplayerGame *game);

/**
 * 게임 렌더링
 */
void mp_render_game(UIManager *ui_mgr, MultiplayerGame *game);

/**
 * 내 턴 입력 처리
 * @return 돌을 놓았으면 true
 */
bool mp_handle_my_turn(UIManager *ui_mgr, MultiplayerGame *game, InputHandler *input_handler);

/**
 * 상대 턴 입력 처리 (채팅/모달만)
 */
void mp_handle_opponent_turn(UIManager *ui_mgr, MultiplayerGame *game, InputHandler *input_handler);

/**
 * 게임 종료 체크 (승리/타임아웃)
 * @param is_host 호스트 여부 (관전자 브로드캐스트용)
 */
void mp_check_game_end(MultiplayerGame *game, bool is_host);

/**
 * 게임 종료 후 입력 처리
 * @return 메인 화면으로 돌아가면 true
 */
bool mp_handle_game_over_input(UIManager *ui_mgr, MultiplayerGame *game, InputHandler *input_handler);

// ============================================================================
// 관전자 관련 함수 (호스트 전용)
// ============================================================================

/**
 * 관전자 연결 처리 (호스트 전용)
 */
void mp_handle_spectator_connections(MultiplayerGame *game);

/**
 * 관전자에게 게임 상태 전송
 */
void mp_send_game_state_to_spectator(MultiplayerGame *game, int spectator_index);

#endif // MP_COMMON_H
