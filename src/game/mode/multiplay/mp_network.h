#ifndef MP_NETWORK_H
#define MP_NETWORK_H

#include "mp_types.h"
#include "../../../ui/core/ui_manager.h"
#include <sys/socket.h>

// ============================================================================
// 네트워크 메시지 처리
// ============================================================================

/**
 * 에러 체크를 포함한 네트워크 메시지 전송
 */
bool mp_send_with_error_check(MultiplayerGame *game, const Message *msg, const char *error_context);

/**
 * 네트워크 메시지 처리
 * @return 메시지가 처리되었으면 true
 */
bool mp_handle_network_messages(UIManager *ui_mgr, MultiplayerGame *game);

// ============================================================================
// 관전자 브로드캐스트
// ============================================================================

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

// ============================================================================
// 관전자 연결 관리 (호스트 전용)
// ============================================================================

/**
 * 관전자에게 게임 상태 전송
 */
void mp_send_game_state_to_spectator(MultiplayerGame *game, int spectator_index);

/**
 * 관전자 연결 처리 (호스트 전용)
 */
void mp_handle_spectator_connections(MultiplayerGame *game);

#endif // MP_NETWORK_H
