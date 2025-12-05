#ifndef MP_INPUT_H
#define MP_INPUT_H

#include "mp_types.h"

// ============================================================================
// 모달 입력 처리
// ============================================================================

/**
 * 공통 모달 입력 처리 (게임패드 지원)
 * @return MpModalResult
 */
MpModalResult mp_handle_modal_input(MultiplayerGame *game, InputAction action);

// ============================================================================
// 채팅 입력 처리
// ============================================================================

/**
 * 채팅 입력 처리 (명령어 포함)
 * @param is_my_turn 내 턴인지 여부 (Undo 가능 여부 결정)
 * @return true if 채팅 모드 종료
 */
bool mp_handle_chat_input(MultiplayerGame *game, int ch, bool is_my_turn);

#endif // MP_INPUT_H
