#ifndef MP_GAME_H
#define MP_GAME_H

#include "mp_types.h"
#include "../../../ui/core/ui_manager.h"
#include "../../../ui/core/input_handler.h"

// ============================================================================
// 게임 초기화/정리
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
// 렌더링
// ============================================================================

/**
 * 게임 렌더링
 */
void mp_render_game(UIManager *ui_mgr, MultiplayerGame *game);

#endif // MP_GAME_H
