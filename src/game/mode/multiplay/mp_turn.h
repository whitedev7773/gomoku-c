// ============================================================================
// mp_turn.h - 턴 처리 헤더
// ============================================================================

#ifndef MP_TURN_H
#define MP_TURN_H

#include "mp_types.h"

// 커서 이동 및 전송
void mp_move_cursor_and_send(MultiplayerGame *game, UIRenderFlags *render_flags, int row_delta, int col_delta);

// 턴 처리
bool mp_handle_my_turn(UIManager *ui_mgr, MultiplayerGame *game, InputHandler *input_handler);
void mp_handle_opponent_turn(UIManager *ui_mgr, MultiplayerGame *game, InputHandler *input_handler);

// 게임 종료 처리
void mp_check_game_end(MultiplayerGame *game, bool is_host);
bool mp_handle_game_over_input(UIManager *ui_mgr, MultiplayerGame *game, InputHandler *input_handler);

#endif // MP_TURN_H
