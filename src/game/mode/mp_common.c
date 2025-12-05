#include "mp_common.h"
#include "../../ui/core/theme.h"
#include "../../ui/game/border/ingame_border.h"
#include <string.h>
#include <sys/socket.h>
#include <ncurses.h>
#include <locale.h>

// ============================================================================
// 모달 입력 처리 (공통)
// ============================================================================

MpModalResult mp_handle_modal_input(MultiplayerGame *game, InputAction action)
{
    if (!modal_ui_is_active(&game->modal_ui) || action == INPUT_NONE)
    {
        return MP_MODAL_NONE;
    }

    ModalResult result = modal_ui_handle_action(&game->modal_ui, action);
    MpModalResult ret = MP_MODAL_NONE;

    switch (result)
    {
    case MODAL_RESULT_YES:
        // 기권 확정
        if (game->modal_ui.type == MODAL_GIVEUP)
        {
            game->game_over = true;
            game->result = (game->me.color == BLACK) ? GAME_WHITE_WIN : GAME_BLACK_WIN;
            modal_ui_close(&game->modal_ui);

            // 기권 메시지 전송
            Message msg;
            protocol_init_message(&msg, MSG_COMMAND, game->network.sequence_number++);
            msg.payload.command.command_type = CMD_GIVEUP;
            mp_send_with_error_check(game, &msg, "sending giveup");

            // 관전자에게 기권 결과 전송
            char giveup_msg[128];
            snprintf(giveup_msg, sizeof(giveup_msg), "%s gave up. %s wins!",
                     game->me.name, game->opponent.name);
            uint8_t giveup_result = (game->result == GAME_BLACK_WIN) ? RESULT_BLACK_WIN : RESULT_WHITE_WIN;
            mp_broadcast_game_result_to_spectators(game, giveup_result, REASON_GIVEUP, game->opponent.name, giveup_msg);

            ret = MP_MODAL_GAME_OVER;
        }
        break;

    case MODAL_RESULT_NO:
    case MODAL_RESULT_CANCEL:
        modal_ui_close(&game->modal_ui);
        ret = MP_MODAL_CLOSED;
        break;

    case MODAL_RESULT_ACCEPT:
        // 무르기/Swap 수락
        if (game->modal_ui.type == MODAL_UNDO_RESPONSE)
        {
            Message msg;
            protocol_init_message(&msg, MSG_COMMAND_RESPONSE, game->network.sequence_number++);
            msg.payload.command_response.command_type = CMD_UNDO;
            msg.payload.command_response.accepted = 1;
            strncpy(msg.payload.command_response.message, "Undo accepted", sizeof(msg.payload.command_response.message) - 1);
            network_send_message(&game->network, &msg);

            // 로컬에서도 Undo 적용
            board_undo_last_move(&game->board);
            board_undo_last_move(&game->board);

            modal_ui_close(&game->modal_ui);
            chat_add_msg(&game->chat_ui, "Undo accepted", CHAT_MSG_SYSTEM);
            game->first_render = true;
            ret = MP_MODAL_CLOSED;
        }
        else if (game->modal_ui.type == MODAL_SWAP_RESPONSE)
        {
            Message msg;
            protocol_init_message(&msg, MSG_COMMAND_RESPONSE, game->network.sequence_number++);
            msg.payload.command_response.command_type = CMD_SWAP;
            msg.payload.command_response.accepted = 1;
            strncpy(msg.payload.command_response.message, "Swap accepted", sizeof(msg.payload.command_response.message) - 1);
            network_send_message(&game->network, &msg);

            // 로컬에서도 Swap 적용
            Stone temp = game->me.color;
            game->me.color = game->opponent.color;
            game->opponent.color = temp;
            game->swap_used = true;

            modal_ui_close(&game->modal_ui);
            chat_add_msg(&game->chat_ui, "Swap accepted", CHAT_MSG_SYSTEM);
            game->first_render = true;
            ret = MP_MODAL_CLOSED;
        }
        break;

    case MODAL_RESULT_DECLINE:
        // 무르기/Swap 거절
        if (game->modal_ui.type == MODAL_UNDO_RESPONSE)
        {
            Message msg;
            protocol_init_message(&msg, MSG_COMMAND_RESPONSE, game->network.sequence_number++);
            msg.payload.command_response.command_type = CMD_UNDO;
            msg.payload.command_response.accepted = 0;
            strncpy(msg.payload.command_response.message, "Undo declined", sizeof(msg.payload.command_response.message) - 1);
            network_send_message(&game->network, &msg);
            chat_add_msg(&game->chat_ui, "Undo declined", CHAT_MSG_SYSTEM);
        }
        else if (game->modal_ui.type == MODAL_SWAP_RESPONSE)
        {
            Message msg;
            protocol_init_message(&msg, MSG_COMMAND_RESPONSE, game->network.sequence_number++);
            msg.payload.command_response.command_type = CMD_SWAP;
            msg.payload.command_response.accepted = 0;
            strncpy(msg.payload.command_response.message, "Swap declined", sizeof(msg.payload.command_response.message) - 1);
            network_send_message(&game->network, &msg);
            chat_add_msg(&game->chat_ui, "Swap declined", CHAT_MSG_SYSTEM);
        }
        modal_ui_close(&game->modal_ui);
        ret = MP_MODAL_CLOSED;
        break;

    case MODAL_RESULT_OK:
        modal_ui_close(&game->modal_ui);
        ret = MP_MODAL_CLOSED;
        break;

    default:
        break;
    }

    // 모달이 닫혔으면 전체 재렌더링
    if (!modal_ui_is_active(&game->modal_ui) && ret != MP_MODAL_NONE)
    {
        game->first_render = true;
    }

    return ret;
}

// ============================================================================
// 채팅 입력 처리 (명령어 포함)
// ============================================================================

bool mp_handle_chat_input(MultiplayerGame *game, int ch, bool is_my_turn)
{
    if (ch == '\n' || ch == KEY_ENTER)
    {
        const char *msg_text = chat_get_msg(&game->chat_ui);
        if (strlen(msg_text) > 0)
        {
            // 명령어 체크
            if (command_is_command(msg_text))
            {
                CommandResult cmd = command_parse(msg_text);
                if (cmd.valid)
                {
                    char modal_msg[MODAL_MAX_MESSAGE_LENGTH];

                    switch (cmd.type)
                    {
                    case CMD_GIVEUP:
                        snprintf(modal_msg, sizeof(modal_msg), "Are you sure you want to give up?");
                        modal_ui_show(&game->modal_ui, MODAL_GIVEUP, modal_msg);
                        break;

                    case CMD_UNDO:
                        if (!is_my_turn)
                        {
                            chat_add_msg(&game->chat_ui, "Undo only on your turn", CHAT_MSG_SYSTEM);
                        }
                        else if (board_get_move_count(&game->board) == 0)
                        {
                            chat_add_msg(&game->chat_ui, "No moves to undo", CHAT_MSG_SYSTEM);
                        }
                        else
                        {
                            Message msg;
                            protocol_init_message(&msg, MSG_COMMAND, game->network.sequence_number++);
                            msg.payload.command.command_type = CMD_UNDO;
                            network_send_message(&game->network, &msg);

                            snprintf(modal_msg, sizeof(modal_msg), "Waiting for opponent's response...");
                            modal_ui_show(&game->modal_ui, MODAL_UNDO_REQUEST, modal_msg);
                            chat_add_msg(&game->chat_ui, "Undo request sent", CHAT_MSG_SYSTEM);
                        }
                        break;

                    case CMD_SWAP:
                        if (game->me.color != WHITE)
                        {
                            chat_add_msg(&game->chat_ui, "Only WHITE can use swap", CHAT_MSG_SYSTEM);
                        }
                        else if (game->swap_used)
                        {
                            chat_add_msg(&game->chat_ui, "Swap already used", CHAT_MSG_SYSTEM);
                        }
                        else if (board_get_move_count(&game->board) < 3)
                        {
                            chat_add_msg(&game->chat_ui, "Swap available after 3 moves", CHAT_MSG_SYSTEM);
                        }
                        else
                        {
                            Message msg;
                            protocol_init_message(&msg, MSG_COMMAND, game->network.sequence_number++);
                            msg.payload.command.command_type = CMD_SWAP;
                            network_send_message(&game->network, &msg);

                            snprintf(modal_msg, sizeof(modal_msg), "Waiting for opponent's response...");
                            modal_ui_show(&game->modal_ui, MODAL_SWAP_REQUEST, modal_msg);
                            chat_add_msg(&game->chat_ui, "Swap request sent", CHAT_MSG_SYSTEM);
                        }
                        break;

                    case CMD_QUIT:
                    {
                        Message msg;
                        protocol_init_message(&msg, MSG_COMMAND, game->network.sequence_number++);
                        msg.payload.command.command_type = CMD_QUIT;
                        network_send_message(&game->network, &msg);
                        game->game_over = true;
                        game->quit_requested = true;
                    }
                    break;

                    case CMD_HELP:
                        chat_add_msg(&game->chat_ui, "=== Commands ===", CHAT_MSG_SYSTEM);
                        chat_add_msg(&game->chat_ui, "/help  - Show help", CHAT_MSG_SYSTEM);
                        chat_add_msg(&game->chat_ui, "/quit  - Leave game", CHAT_MSG_SYSTEM);
                        chat_add_msg(&game->chat_ui, "/undo  - Request undo", CHAT_MSG_SYSTEM);
                        chat_add_msg(&game->chat_ui, "/giveup - Forfeit", CHAT_MSG_SYSTEM);
                        chat_add_msg(&game->chat_ui, "/swap  - Swap colors", CHAT_MSG_SYSTEM);
                        break;

                    default:
                        break;
                    }
                }
                else
                {
                    chat_add_msg(&game->chat_ui, cmd.error_message, CHAT_MSG_SYSTEM);
                }
            }
            else
            {
                // 일반 채팅 전송
                Message msg;
                protocol_init_message(&msg, MSG_CHAT, game->network.sequence_number++);
                strncpy(msg.payload.chat.message, msg_text, sizeof(msg.payload.chat.message) - 1);
                network_send_message(&game->network, &msg);

                mp_broadcast_chat_to_spectators(game, &msg);
                chat_add_msg(&game->chat_ui, msg_text, CHAT_MSG_USER);
            }
        }
        chat_exit_input(&game->chat_ui);
        return true;
    }
    else
    {
        chat_handle_input(&game->chat_ui, ch);
        return false;
    }
}

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
// 관전자 브로드캐스트 함수들
// ============================================================================

void mp_broadcast_cursor_to_spectators(MultiplayerGame *game, int row, int col)
{
    Message msg;
    protocol_init_message(&msg, MSG_CURSOR_UPDATE, game->network.sequence_number++);
    msg.payload.cursor.row = row;
    msg.payload.cursor.col = col;
    network_broadcast_to_spectators(&game->network, &msg);
}

void mp_broadcast_game_result_to_spectators(MultiplayerGame *game, uint8_t result_type, uint8_t reason, const char *winner_name, const char *message)
{
    Message msg;
    protocol_init_message(&msg, MSG_GAME_RESULT, game->network.sequence_number++);
    msg.payload.game_result.result_type = result_type;
    msg.payload.game_result.reason = reason;
    if (winner_name)
        strncpy(msg.payload.game_result.winner_name, winner_name, MAX_PLAYER_NAME - 1);
    else
        msg.payload.game_result.winner_name[0] = '\0';
    if (message)
        strncpy(msg.payload.game_result.message, message, sizeof(msg.payload.game_result.message) - 1);
    else
        msg.payload.game_result.message[0] = '\0';
    network_broadcast_to_spectators(&game->network, &msg);
}

void mp_broadcast_move_to_spectators(MultiplayerGame *game, const Message *move_msg)
{
    network_broadcast_to_spectators(&game->network, move_msg);
}

void mp_broadcast_chat_to_spectators(MultiplayerGame *game, const Message *chat_msg)
{
    network_broadcast_to_spectators(&game->network, chat_msg);
}

// ============================================================================
// 에러 체크 포함 네트워크 전송
// ============================================================================

bool mp_send_with_error_check(MultiplayerGame *game, const Message *msg, const char *error_context)
{
    int result = network_send_message(&game->network, msg);

    if (result < 0)
    {
        if (!network_is_connected(&game->network) ||
            game->network.state == NET_DISCONNECTED ||
            game->network.state == NET_ERROR)
        {
            if (!game->game_over)
            {
                char modal_msg[MODAL_MAX_MESSAGE_LENGTH];
                snprintf(modal_msg, sizeof(modal_msg),
                         "Connection lost while %s. Game will end.",
                         error_context ? error_context : "sending data");
                modal_ui_show(&game->modal_ui, MODAL_ERROR, modal_msg);

                char log_msg[128];
                snprintf(log_msg, sizeof(log_msg), "Connection lost!");
                log_add_msg(&game->log_ui, log_msg);
                chat_add_msg(&game->chat_ui, "Connection lost", CHAT_MSG_SYSTEM);

                game->game_over = true;
            }
        }
        return false;
    }

    return true;
}

// ============================================================================
// 게임 초기화/정리 함수
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

    return true;
}

void mp_cleanup_game(UIManager *ui_mgr, MultiplayerGame *game, InputHandler *input_handler)
{
    input_handler_cleanup(input_handler);
    logger_close(&game->logger);
    ui_manager_cleanup(ui_mgr);
    endwin();
    network_cleanup(&game->network);
}

// ============================================================================
// 네트워크 메시지 처리
// ============================================================================

bool mp_handle_network_messages(UIManager *ui_mgr, MultiplayerGame *game)
{
    Message msg;
    int result = network_receive_message(&game->network, &msg);

    if (result < 0)
    {
        if (!network_is_connected(&game->network) ||
            game->network.state == NET_DISCONNECTED ||
            game->network.state == NET_ERROR)
        {
            if (!game->game_over)
            {
                char modal_msg[MODAL_MAX_MESSAGE_LENGTH];
                snprintf(modal_msg, sizeof(modal_msg),
                         "Connection lost with %s. Game will end.",
                         game->opponent.name);
                modal_ui_show(&game->modal_ui, MODAL_ERROR, modal_msg);
                log_add_msg(&game->log_ui, "Connection lost!");
                chat_add_msg(&game->chat_ui, "Connection lost", CHAT_MSG_SYSTEM);
                game->game_over = true;
            }
        }
        return false;
    }

    if (result == 0)
    {
        return false;
    }

    switch (msg.header.type)
    {
    case MSG_MOVE:
    {
        int prev_last_row = game->board.last_row;
        int prev_last_col = game->board.last_col;

        if (board_place_stone(&game->board, msg.payload.move.row, msg.payload.move.col,
                              msg.payload.move.stone))
        {
            char move_msg_str[128];
            snprintf(move_msg_str, sizeof(move_msg_str), "%s placed at %c%02d",
                     game->opponent.name,
                     board_col_to_char(msg.payload.move.col),
                     msg.payload.move.row + 1);
            log_add_msg(&game->log_ui, move_msg_str);

            logger_log_move(&game->logger, msg.payload.move.stone,
                            msg.payload.move.row, msg.payload.move.col,
                            board_get_move_count(&game->board));

            if (prev_last_row >= 0 && prev_last_col >= 0)
            {
                ui_render_flags_add_dirty_cell(&ui_mgr->render_flags, prev_last_row, prev_last_col);
            }
            ui_render_flags_add_dirty_cell(&ui_mgr->render_flags, msg.payload.move.row, msg.payload.move.col);

            turn_manager_next_turn(&game->turn_mgr);
            ui_render_flags_set(&ui_mgr->render_flags, RENDER_BOARD_FULL);

            mp_broadcast_move_to_spectators(game, &msg);
        }
    }
    break;

    case MSG_CURSOR_UPDATE:
        board_update_opponent_cursor(&game->opponent_cursor,
                                     msg.payload.cursor.row,
                                     msg.payload.cursor.col,
                                     &ui_mgr->render_flags);
        if (game->network.role == NETWORK_SERVER)
        {
            mp_broadcast_cursor_to_spectators(game, msg.payload.cursor.row, msg.payload.cursor.col);
        }
        break;

    case MSG_CHAT:
        chat_add_msg(&game->chat_ui, msg.payload.chat.message, CHAT_MSG_OPPONENT);
        mp_broadcast_chat_to_spectators(game, &msg);
        break;

    case MSG_COMMAND:
    {
        CommandType cmd_type = msg.payload.command.command_type;
        char modal_msg[MODAL_MAX_MESSAGE_LENGTH];

        switch (cmd_type)
        {
        case CMD_QUIT:
            snprintf(modal_msg, sizeof(modal_msg), "%s has quit the game.", game->opponent.name);
            modal_ui_show(&game->modal_ui, MODAL_ERROR, modal_msg);
            game->game_over = true;
            {
                uint8_t quit_result = (game->me.color == BLACK) ? RESULT_BLACK_WIN : RESULT_WHITE_WIN;
                mp_broadcast_game_result_to_spectators(game, quit_result, REASON_QUIT, game->me.name, modal_msg);
            }
            break;

        case CMD_GIVEUP:
            game->game_over = true;
            game->result = (game->opponent.color == BLACK) ? GAME_WHITE_WIN : GAME_BLACK_WIN;
            snprintf(modal_msg, sizeof(modal_msg), "%s has given up!", game->opponent.name);
            modal_ui_show(&game->modal_ui, MODAL_GAME_RESULT, modal_msg);
            chat_add_msg(&game->chat_ui, modal_msg, CHAT_MSG_SYSTEM);
            {
                uint8_t giveup_result = (game->result == GAME_BLACK_WIN) ? RESULT_BLACK_WIN : RESULT_WHITE_WIN;
                const char *winner = (game->result == GAME_BLACK_WIN)
                                         ? ((game->me.color == BLACK) ? game->me.name : game->opponent.name)
                                         : ((game->me.color == WHITE) ? game->me.name : game->opponent.name);
                mp_broadcast_game_result_to_spectators(game, giveup_result, REASON_GIVEUP, winner, modal_msg);
            }
            break;

        case CMD_UNDO:
            snprintf(modal_msg, sizeof(modal_msg), "%s wants to undo the last move", game->opponent.name);
            modal_ui_show(&game->modal_ui, MODAL_UNDO_RESPONSE, modal_msg);
            break;

        case CMD_SWAP:
            snprintf(modal_msg, sizeof(modal_msg), "%s wants to swap colors", game->opponent.name);
            modal_ui_show(&game->modal_ui, MODAL_SWAP_RESPONSE, modal_msg);
            break;

        default:
            break;
        }
    }
    break;

    case MSG_COMMAND_RESPONSE:
    {
        uint8_t cmd_type = msg.payload.command_response.command_type;
        bool accepted = msg.payload.command_response.accepted;

        if (modal_ui_is_active(&game->modal_ui) &&
            (game->modal_ui.type == MODAL_UNDO_REQUEST || game->modal_ui.type == MODAL_SWAP_REQUEST))
        {
            modal_ui_close(&game->modal_ui);
            game->first_render = true;
        }

        if (cmd_type == CMD_UNDO)
        {
            if (accepted)
            {
                board_undo_last_move(&game->board);
                board_undo_last_move(&game->board);
                chat_add_msg(&game->chat_ui, "Undo accepted by opponent", CHAT_MSG_SYSTEM);
                game->first_render = true;
                ui_render_flags_set(&ui_mgr->render_flags, RENDER_BOARD_FULL);
                ui_render_flags_set(&ui_mgr->render_flags, RENDER_INFO);
            }
            else
            {
                chat_add_msg(&game->chat_ui, "Undo rejected by opponent", CHAT_MSG_SYSTEM);
            }
        }
        else if (cmd_type == CMD_SWAP)
        {
            if (accepted)
            {
                Stone temp = game->me.color;
                game->me.color = game->opponent.color;
                game->opponent.color = temp;
                game->swap_used = true;
                chat_add_msg(&game->chat_ui, "Swap accepted by opponent", CHAT_MSG_SYSTEM);
                game->first_render = true;
                ui_render_flags_set(&ui_mgr->render_flags, RENDER_INFO);
            }
            else
            {
                chat_add_msg(&game->chat_ui, "Swap rejected by opponent", CHAT_MSG_SYSTEM);
            }
        }
    }
    break;

    default:
        break;
    }

    return true;
}

// ============================================================================
// UI 렌더링
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

    game->first_render = false;

    if (modal_ui_is_active(&game->modal_ui))
    {
        modal_ui_render(stdscr, &game->modal_ui);
    }
}

// ============================================================================
// 내 턴 처리
// ============================================================================

bool mp_handle_my_turn(UIManager *ui_mgr, MultiplayerGame *game, InputHandler *input_handler)
{
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

// ============================================================================
// 관전자 관련 함수들
// ============================================================================

void mp_send_game_state_to_spectator(MultiplayerGame *game, int spectator_index)
{
    Message msg;
    protocol_init_message(&msg, MSG_GAME_STATE, game->network.sequence_number++);

    strncpy(msg.payload.game_state.player1_name, game->me.name, MAX_PLAYER_NAME);
    strncpy(msg.payload.game_state.player2_name, game->opponent.name, MAX_PLAYER_NAME);
    msg.payload.game_state.current_turn = turn_manager_get_current_player(&game->turn_mgr);
    msg.payload.game_state.move_count = board_get_move_count(&game->board);

    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            msg.payload.game_state.board_state[row * BOARD_SIZE + col] = game->board.cells[row][col];
        }
    }

    network_send_to_spectator(&game->network, spectator_index, &msg);
}

void mp_handle_spectator_connections(MultiplayerGame *game)
{
    if (network_server_accept_spectator(&game->network))
    {
        int new_spectator_index = game->network.spectator_count - 1;

        uint8_t temp_buffer[1024];
        int spectator_fd = game->network.spectator_fds[new_spectator_index];

        ssize_t received = recv(spectator_fd, temp_buffer, sizeof(temp_buffer), MSG_DONTWAIT);
        if (received > 0)
        {
            Message connect_msg;
            int result = protocol_deserialize(&connect_msg, temp_buffer, received);

            if (result > 0 && connect_msg.header.type == MSG_SPECTATOR_CONNECT)
            {
                strncpy(game->network.spectator_names[new_spectator_index],
                        connect_msg.payload.spectator_connect.spectator_name,
                        MAX_PLAYER_NAME);

                Message ack_msg;
                protocol_init_message(&ack_msg, MSG_SPECTATOR_CONNECT_ACK, game->network.sequence_number++);
                ack_msg.payload.spectator_connect_ack.accepted = 1;
                ack_msg.payload.spectator_connect_ack.error_code = ERR_NONE;
                ack_msg.payload.spectator_connect_ack.spectator_count = game->network.spectator_count;
                ack_msg.payload.spectator_connect_ack.max_spectators = MAX_SPECTATORS;
                network_send_to_spectator(&game->network, new_spectator_index, &ack_msg);

                mp_send_game_state_to_spectator(game, new_spectator_index);

                Message join_msg;
                protocol_init_message(&join_msg, MSG_SPECTATOR_JOIN, game->network.sequence_number++);
                strncpy(join_msg.payload.spectator_join_leave.spectator_name,
                        game->network.spectator_names[new_spectator_index],
                        MAX_PLAYER_NAME);
                join_msg.payload.spectator_join_leave.spectator_count = game->network.spectator_count;

                network_send_message(&game->network, &join_msg);
                for (int i = 0; i < MAX_SPECTATORS; i++)
                {
                    if (i != new_spectator_index && game->network.spectator_fds[i] >= 0)
                    {
                        network_send_to_spectator(&game->network, i, &join_msg);
                    }
                }
            }
        }
    }
}
