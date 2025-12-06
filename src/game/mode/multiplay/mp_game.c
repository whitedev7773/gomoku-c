// ============================================================================
// mp_game.c - 멀티플레이 게임 초기화/정리/렌더링
// ============================================================================

#include "mp_game.h"
#include "../../../ui/core/theme.h"
#include "../../../ui/game/border/ingame_border.h"
#include "../../../utils/terminal_check.h"
#include <string.h>
#include <ncurses.h>
#include <locale.h>

// ============================================================================
// 게임 초기화
// ============================================================================

bool mp_init_game_ui(UIManager *ui_mgr, MultiplayerGame *game, GameRule rule)
{
    // UTF-8 로케일 설정 (ncurses 초기화 전에 반드시 호출)
    setlocale(LC_ALL, "");

    // ui_manager_init이 ncurses 초기화를 담당
    if (!ui_manager_init(ui_mgr))
    {
        endwin();
        return false;
    }

    ingame_border_draw();
    refresh();

    // 게임 컴포넌트 초기화
    board_init_with_rule(&game->board, rule);
    board_init_cursor(&game->my_cursor);
    board_init_cursor(&game->opponent_cursor);
    turn_manager_init(&game->turn_mgr, BLACK);
    game_info_ui_init(&game->info_ui);
    log_init(&game->log_ui);
    chat_init(&game->chat_ui);
    modal_ui_init(&game->modal_ui);

    // 로거 초기화
    if (!logger_init(&game->logger))
    {
        char error_msg[MODAL_MAX_MESSAGE_LENGTH];
        snprintf(error_msg, sizeof(error_msg),
                 "Failed to create game log file. The game will continue without logging.");
        modal_ui_show(&game->modal_ui, MODAL_ERROR, error_msg);
        log_add_msg(&game->log_ui, "Warning: Game logging disabled");
    }

    keypad(ui_mgr->board_win, TRUE);
    wtimeout(ui_mgr->board_win, 50);

    game->first_render = true;
    game->terminal_warning_shown = false;

    return true;
}

// ============================================================================
// 게임 정리
// ============================================================================

void mp_cleanup_game(UIManager *ui_mgr, MultiplayerGame *game, InputHandler *input_handler)
{
    input_handler_cleanup(input_handler);
    logger_close(&game->logger);
    ui_manager_cleanup(ui_mgr);
    endwin();
    network_cleanup(&game->network);
}

// ============================================================================
// 게임 렌더링
// ============================================================================

void mp_render_game(UIManager *ui_mgr, MultiplayerGame *game)
{
    Stone current_player = turn_manager_get_current_player(&game->turn_mgr);
    if (current_player == BLACK)
    {
        board_update_forbidden_marks(&game->board, BLACK);
    }

    UIRenderFlags *render_flags = &ui_mgr->render_flags;

    ui_render_flags_set(render_flags, RENDER_TIMER);
    ui_render_flags_set(render_flags, RENDER_PLAY_TIME);
    ui_render_flags_set(render_flags, RENDER_CURRENT_TURN);

    // 상단 Info 영역 렌더링
    if (game->first_render || ui_render_flags_is_set(render_flags, RENDER_INFO))
    {
        game_info_draw_opponent_name(game->opponent.name);
        game_info_draw_viewers(game->network.spectator_count);
        game_info_draw_ping(network_get_ping_ms((NetworkManager *)&game->network));
        game_info_draw_port(game->network.port > 0 ? game->network.port : game->network.remote_port);
    }

    bool is_my_turn = (current_player == game->me.color);

    board_render_mp(ui_mgr->board_win, &game->board,
                    &game->my_cursor, &game->opponent_cursor,
                    render_flags, game->first_render, is_my_turn);

    game_info_ui_selective_render(ui_mgr->bottom_win, &game->board, &game->turn_mgr,
                                  &game->info_ui, render_flags, game->first_render);

    chat_selective_render(ui_mgr->chat_win, &game->chat_ui, render_flags, game->first_render);

    chat_selective_render_input(ui_mgr->chat_input_win, &game->chat_ui,
                                render_flags, game->first_render, 1, 1);

    // System Log 렌더링
    if (game->first_render)
    {
        wrefresh(ui_mgr->system_log_win);
    }
    log_render_sel(ui_mgr->system_log_win, &game->log_ui, render_flags, game->first_render, 0, 0);

    game->first_render = false;

    if (modal_ui_is_active(&game->modal_ui))
    {
        modal_ui_render(stdscr, &game->modal_ui);
    }
}
