#ifndef GAME_INFO_UI_H
#define GAME_INFO_UI_H

// ============================================
// 호환성 래퍼 헤더
// 새 코드에서는 info/top/info_top_ui.h와 info/bottom/info_bottom_ui.h를 직접 사용하세요
// ============================================

#include "info/top/info_top_ui.h"
#include "info/bottom/info_bottom_ui.h"

// 타입 별칭 (호환성 유지)
typedef InfoBottomUI GameInfoUI;

// 함수 별칭 매크로 (호환성 유지)
#define game_info_ui_init info_btm_init
#define game_info_ui_init_bottom info_btm_init_win
#define game_info_ui_update_bottom info_btm_update
#define game_info_ui_draw_last_move info_btm_draw_last_mv
#define game_info_ui_draw_current_turn info_btm_draw_turn
#define game_info_ui_draw_timer info_btm_draw_timer
#define game_info_ui_draw_play_time info_btm_draw_playtime
#define game_info_ui_draw_buttons info_btm_draw_btns
#define game_info_ui_get_elapsed_seconds info_btm_elapsed_sec
#define game_info_ui_selective_render info_btm_render

// 상단 Info 함수 별칭 (호환성 유지)
#define game_info_draw_opponent_name info_top_opponent
#define game_info_draw_viewers info_top_viewers
#define game_info_draw_ping info_top_ping
#define game_info_draw_port info_top_port

#endif // GAME_INFO_UI_H
