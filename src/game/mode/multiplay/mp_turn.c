// ============================================================================
// mp_turn.c - 턴 처리
// ============================================================================

#include "mp_turn.h"
#include "mp_input.h"
#include "mp_network.h"
#include <string.h>

// ============================================================================
// 커서 이동 및 네트워크 전송
// ============================================================================

void mp_move_cursor_and_send(MultiplayerGame *game, UIRenderFlags *render_flags, int row_delta, int col_delta)
{
    board_move_cursor_f(&game->my_cursor, row_delta, col_delta, render_flags);

    Message msg;
    protocol_init_message(&msg, MSG_CURSOR_UPDATE, game->network.sequence_number++);
    msg.payload.cursor.row = game->my_cursor.cursor_row;
    msg.payload.cursor.col = game->my_cursor.cursor_col;
    network_send_message(&game->network, &msg);

    mp_broadcast_cursor_to_spectators(game, game->my_cursor.cursor_row, game->my_cursor.cursor_col);
}

// ============================================================================
// 내 턴 처리
// ============================================================================

bool mp_handle_my_turn(UIManager *ui_mgr, MultiplayerGame *game, InputHandler *input_handler)
{
    // PING 전송 (주기적)
    mp_send_ping_if_needed(game);

    // 모달 처리
    if (modal_ui_is_active(&game->modal_ui))
    {
        InputEvent event = input_handler_get_event(input_handler, ui_mgr->board_win);
        if (event.action != INPUT_NONE)
        {
            mp_handle_modal_input(game, event.action);
        }
        return false;
    }

    // 채팅 모드
    if (chat_is_input_mode(&game->chat_ui))
    {
        InputEvent event = input_handler_get_event(input_handler, ui_mgr->board_win);
        int ch = event.key_code;
        if (ch != ERR)
        {
            mp_handle_chat_input(game, ch, true);
        }
        return false;
    }

    InputEvent event = input_handler_get_event(input_handler, ui_mgr->board_win);

    if (event.action == INPUT_NONE)
    {
        return false;
    }

    if (event.key_code == '\n' || event.key_code == 't' || event.key_code == 'T')
    {
        chat_enter_input(&game->chat_ui);
        return false;
    }

    switch (event.action)
    {
    case INPUT_MOVE_UP:
        mp_move_cursor_and_send(game, &ui_mgr->render_flags, -1, 0);
        break;
    case INPUT_MOVE_DOWN:
        mp_move_cursor_and_send(game, &ui_mgr->render_flags, 1, 0);
        break;
    case INPUT_MOVE_LEFT:
        mp_move_cursor_and_send(game, &ui_mgr->render_flags, 0, -1);
        break;
    case INPUT_MOVE_RIGHT:
        mp_move_cursor_and_send(game, &ui_mgr->render_flags, 0, 1);
        break;
    case INPUT_PLACE_STONE:
        if (board_is_empty(&game->board, game->my_cursor.cursor_row, game->my_cursor.cursor_col))
        {
            if (game->me.color == BLACK && board_is_forbidden(&game->board, game->my_cursor.cursor_row, game->my_cursor.cursor_col))
            {
                chat_add_msg(&game->chat_ui, "Forbidden move! (Renju Rule)", CHAT_MSG_SYSTEM);
            }
            else
            {
                int prev_last_row = game->board.last_row;
                int prev_last_col = game->board.last_col;

                if (board_place_stone(&game->board, game->my_cursor.cursor_row,
                                      game->my_cursor.cursor_col, game->me.color))
                {
                    char move_msg[128];
                    snprintf(move_msg, sizeof(move_msg), "You placed at %c%02d",
                             board_col_to_char(game->my_cursor.cursor_col),
                             game->my_cursor.cursor_row + 1);
                    log_add_msg(&game->log_ui, move_msg);

                    logger_log_move(&game->logger, game->me.color,
                                    game->my_cursor.cursor_row, game->my_cursor.cursor_col,
                                    board_get_move_count(&game->board));

                    if (prev_last_row >= 0 && prev_last_col >= 0)
                    {
                        ui_render_flags_add_dirty_cell(&ui_mgr->render_flags, prev_last_row, prev_last_col);
                    }
                    ui_render_flags_add_dirty_cell(&ui_mgr->render_flags,
                                                   game->my_cursor.cursor_row, game->my_cursor.cursor_col);
                    ui_render_flags_set(&ui_mgr->render_flags, RENDER_LAST_MOVE);
                    ui_render_flags_set(&ui_mgr->render_flags, RENDER_CURRENT_TURN);
                    ui_render_flags_set(&ui_mgr->render_flags, RENDER_INFO);

                    Message msg;
                    protocol_init_message(&msg, MSG_MOVE, game->network.sequence_number++);
                    msg.payload.move.row = game->my_cursor.cursor_row;
                    msg.payload.move.col = game->my_cursor.cursor_col;
                    msg.payload.move.stone = game->me.color;

                    if (!mp_send_with_error_check(game, &msg, "sending move"))
                    {
                        return false;
                    }

                    mp_broadcast_move_to_spectators(game, &msg);

                    return true;
                }
            }
        }
        else
        {
            log_add_msg(&game->log_ui, "Position already occupied!");
        }
        break;
    case INPUT_QUIT:
        break;
    default:
        break;
    }

    return false;
}

// ============================================================================
// 상대 턴 처리
// ============================================================================

void mp_handle_opponent_turn(UIManager *ui_mgr, MultiplayerGame *game, InputHandler *input_handler)
{
    // PING 전송 (주기적)
    mp_send_ping_if_needed(game);

    // 모달 처리
    if (modal_ui_is_active(&game->modal_ui))
    {
        InputEvent event = input_handler_get_event(input_handler, ui_mgr->board_win);
        if (event.action != INPUT_NONE)
        {
            mp_handle_modal_input(game, event.action);
        }
        return;
    }

    // 채팅 모드
    if (chat_is_input_mode(&game->chat_ui))
    {
        InputEvent event = input_handler_get_event(input_handler, ui_mgr->board_win);
        int ch = event.key_code;
        if (ch != ERR)
        {
            mp_handle_chat_input(game, ch, false);
        }
        return;
    }

    InputEvent event = input_handler_get_event(input_handler, ui_mgr->board_win);

    if (event.key_code == '\n' || event.key_code == 't' || event.key_code == 'T')
    {
        chat_enter_input(&game->chat_ui);
    }
}

// ============================================================================
// 게임 종료 체크
// ============================================================================

void mp_check_game_end(MultiplayerGame *game, bool is_host)
{
    if (game->game_over)
        return;

    // 승리 체크
    game->result = game_check_winner(&game->board);
    if (game->result != GAME_ONGOING)
    {
        game->game_over = true;
        char result_msg[128];
        const char *winner_name = NULL;
        uint8_t result_type = RESULT_DRAW;

        if (is_host)
        {
            // 호스트: 이름 포함 메시지
            if (game->result == GAME_BLACK_WIN)
            {
                snprintf(result_msg, sizeof(result_msg), "%s (BLACK) WINS!", game->me.name);
                winner_name = game->me.name;
                result_type = RESULT_BLACK_WIN;
            }
            else if (game->result == GAME_WHITE_WIN)
            {
                snprintf(result_msg, sizeof(result_msg), "%s (WHITE) WINS!", game->opponent.name);
                winner_name = game->opponent.name;
                result_type = RESULT_WHITE_WIN;
            }
            else
            {
                snprintf(result_msg, sizeof(result_msg), "Game ended in a DRAW!");
            }
            mp_broadcast_game_result_to_spectators(game, result_type, REASON_FIVE_IN_A_ROW, winner_name, result_msg);
        }
        else
        {
            // 클라이언트: 승패 메시지
            if ((game->result == GAME_BLACK_WIN && game->me.color == BLACK) ||
                (game->result == GAME_WHITE_WIN && game->me.color == WHITE))
            {
                snprintf(result_msg, sizeof(result_msg), "You WIN!");
            }
            else if (game->result != GAME_DRAW)
            {
                snprintf(result_msg, sizeof(result_msg), "You LOSE!");
            }
            else
            {
                snprintf(result_msg, sizeof(result_msg), "DRAW!");
            }
        }

        log_add_msg(&game->log_ui, result_msg);
        log_add_msg(&game->log_ui, "Press 'q' to quit");
        modal_ui_show(&game->modal_ui, MODAL_GAME_RESULT, result_msg);
        logger_close(&game->logger);
        return;
    }

    // 턴 타임아웃 체크
    if (turn_manager_is_timeout(&game->turn_mgr))
    {
        Stone timed_out_player = turn_manager_get_current_player(&game->turn_mgr);
        game->game_over = true;
        game->result = (timed_out_player == BLACK) ? GAME_WHITE_WIN : GAME_BLACK_WIN;

        char timeout_msg[128];
        if (is_host)
        {
            const char *loser_name = (timed_out_player == game->me.color) ? game->me.name : game->opponent.name;
            const char *winner_name = (timed_out_player == game->me.color) ? game->opponent.name : game->me.name;
            snprintf(timeout_msg, sizeof(timeout_msg), "%s ran out of time! %s wins!", loser_name, winner_name);

            uint8_t timeout_result = (game->result == GAME_BLACK_WIN) ? RESULT_BLACK_WIN : RESULT_WHITE_WIN;
            mp_broadcast_game_result_to_spectators(game, timeout_result, REASON_TIMEOUT, winner_name, timeout_msg);
        }
        else
        {
            if (timed_out_player == game->me.color)
            {
                snprintf(timeout_msg, sizeof(timeout_msg), "Time's up! You LOSE!");
            }
            else
            {
                snprintf(timeout_msg, sizeof(timeout_msg), "Opponent ran out of time! You WIN!");
            }
        }

        modal_ui_show(&game->modal_ui, MODAL_GAME_RESULT, timeout_msg);
        chat_add_msg(&game->chat_ui, timeout_msg, CHAT_MSG_SYSTEM);
        log_add_msg(&game->log_ui, timeout_msg);
        logger_close(&game->logger);
    }
}

// ============================================================================
// 게임 종료 후 입력 처리
// ============================================================================

bool mp_handle_game_over_input(UIManager *ui_mgr, MultiplayerGame *game, InputHandler *input_handler)
{
    if (game->quit_requested)
    {
        return true;
    }

    InputEvent event = input_handler_get_event(input_handler, ui_mgr->board_win);

    if (modal_ui_is_active(&game->modal_ui) && event.action != INPUT_NONE)
    {
        ModalResult result = modal_ui_handle_action(&game->modal_ui, event.action);
        if (result == MODAL_RESULT_OK || result == MODAL_RESULT_CANCEL)
        {
            modal_ui_close(&game->modal_ui);
            return true;
        }
    }

    if (event.action == INPUT_QUIT)
    {
        return true;
    }

    return false;
}
