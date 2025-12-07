// ============================================================================
// mp_network.c - 네트워크 메시지 처리 및 브로드캐스트
// ============================================================================

#include "mp_network.h"
#include <string.h>
#include <sys/time.h>

// ============================================================================
// 헬퍼 함수
// ============================================================================

static uint64_t get_current_time_ms();

// ============================================================================
// 에러 체크 포함 네트워크 전송
// ============================================================================

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

            // 턴 변경 후 System Log에 메시지
            Stone next_player = turn_manager_get_current_player(&game->turn_mgr);
            char turn_msg[32];
            const char *player_name = (next_player == game->me.color) ? game->me.name : game->opponent.name;
            snprintf(turn_msg, sizeof(turn_msg), "%s's turn", player_name);
            log_add_msg(&game->log_ui, turn_msg);
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
    {
        // 게임 경과 시간 계산 (초 단위)
        int game_time_seconds = (int)difftime(time(NULL), game->game_start_time);
        chat_add_msg_with_time(&game->chat_ui, msg.payload.chat.message, CHAT_MSG_OPPONENT, game_time_seconds);
        mp_broadcast_chat_to_spectators(game, &msg);
    }
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
            snprintf(modal_msg, sizeof(modal_msg), "%s wants to give up", game->opponent.name);
            modal_ui_show(&game->modal_ui, MODAL_GIVEUP_RESPONSE, modal_msg);
            break;

        case CMD_UNDO:
            snprintf(modal_msg, sizeof(modal_msg), "%s wants to undo the last move", game->opponent.name);
            modal_ui_show(&game->modal_ui, MODAL_UNDO_RESPONSE, modal_msg);
            break;

        case CMD_SWAP:
            if (game->opponent.color == WHITE)
            {
                // 화이트가 요청하면 즉시 수락
                Stone temp = game->me.color;
                game->me.color = game->opponent.color;
                game->opponent.color = temp;
                game->swap_used = true;
                chat_add_msg(&game->chat_ui, "Swap accepted automatically (WHITE requested)", CHAT_MSG_SYSTEM);
                game->first_render = true;
                ui_render_flags_set(&ui_mgr->render_flags, RENDER_INFO);

                // 응답 전송
                Message response_msg;
                protocol_init_message(&response_msg, MSG_COMMAND_RESPONSE, game->network.sequence_number++);
                response_msg.payload.command_response.command_type = CMD_SWAP;
                response_msg.payload.command_response.accepted = 1;
                strncpy(response_msg.payload.command_response.message, "Swap accepted automatically", sizeof(response_msg.payload.command_response.message) - 1);
                mp_send_with_error_check(game, &response_msg, "sending swap response");
            }
            else
            {
                // 블랙이 요청하면 모달
                snprintf(modal_msg, sizeof(modal_msg), "%s wants to swap colors", game->opponent.name);
                modal_ui_show(&game->modal_ui, MODAL_SWAP_RESPONSE, modal_msg);
            }
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
            (game->modal_ui.type == MODAL_UNDO_REQUEST || game->modal_ui.type == MODAL_SWAP_REQUEST || game->modal_ui.type == MODAL_GIVEUP_REQUEST))
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
        else if (cmd_type == CMD_GIVEUP)
        {
            if (accepted)
            {
                game->game_over = true;
                game->result = (game->me.color == BLACK) ? GAME_WHITE_WIN : GAME_BLACK_WIN;
                char giveup_msg[128];
                snprintf(giveup_msg, sizeof(giveup_msg), "%s gave up. %s wins!",
                         game->me.name, game->opponent.name);
                modal_ui_show(&game->modal_ui, MODAL_GAME_RESULT, giveup_msg);
                chat_add_msg(&game->chat_ui, giveup_msg, CHAT_MSG_SYSTEM);

                // 관전자에게 기권 결과 전송
                uint8_t giveup_result = (game->result == GAME_BLACK_WIN) ? RESULT_BLACK_WIN : RESULT_WHITE_WIN;
                mp_broadcast_game_result_to_spectators(game, giveup_result, REASON_GIVEUP, game->opponent.name, giveup_msg);
            }
            else
            {
                chat_add_msg(&game->chat_ui, "Giveup rejected by opponent", CHAT_MSG_SYSTEM);
            }
        }
    }
    break;

    case MSG_SPECTATOR_JOIN:
    {
        // 관전자 입장 알림 (클라이언트 측에서 수신)
        game->network.spectator_count = msg.payload.spectator_join_leave.spectator_count;
        char sys_msg[128];
        snprintf(sys_msg, sizeof(sys_msg), "[Viewer] %s joined (%d)",
                 msg.payload.spectator_join_leave.spectator_name,
                 game->network.spectator_count);
        chat_add_msg(&game->chat_ui, sys_msg, CHAT_MSG_SYSTEM);
        ui_render_flags_set(&ui_mgr->render_flags, RENDER_INFO);
    }
    break;

    case MSG_SPECTATOR_LEAVE:
    {
        // 관전자 퇴장 알림 (클라이언트 측에서 수신)
        game->network.spectator_count = msg.payload.spectator_join_leave.spectator_count;
        char sys_msg[128];
        snprintf(sys_msg, sizeof(sys_msg), "[Viewer] %s left (%d)",
                 msg.payload.spectator_join_leave.spectator_name,
                 game->network.spectator_count);
        chat_add_msg(&game->chat_ui, sys_msg, CHAT_MSG_SYSTEM);
        ui_render_flags_set(&ui_mgr->render_flags, RENDER_INFO);
    }
    break;

    case MSG_PING:
    {
        // PING 수신: PONG 응답
        Message pong_msg;
        protocol_init_message(&pong_msg, MSG_PONG, game->network.sequence_number++);
        pong_msg.payload.ping_pong.timestamp = msg.payload.ping_pong.timestamp;
        mp_send_with_error_check(game, &pong_msg, "sending pong");
    }
    break;

    case MSG_PONG:
    {
        // PONG 수신: RTT 계산
        if (game->network.ping_pending)
        {
            uint64_t now = get_current_time_ms();
            int rtt = (int)(now - game->network.ping_last_sent);
            game->network.ping_rtt_ms = rtt;
            game->network.ping_pending = false;
            // PING 값이 변경되었으므로 UI 업데이트 플래그 설정
            ui_render_flags_set(&ui_mgr->render_flags, RENDER_INFO);
        }
    }
    break;

    default:
        break;
    }

    return true;
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
// 관전자 연결 관리 (호스트 전용)
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
    int new_spectator_index = network_server_accept_spectator(&game->network);
    if (new_spectator_index >= 0)
    {
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

    // 기존 관전자들의 연결 상태 체크
    for (int i = 0; i < MAX_SPECTATORS; i++)
    {
        if (game->network.spectator_fds[i] >= 0)
        {
            // 연결 상태 확인을 위한 peek
            char peek_buf[1];
            ssize_t peek_result = recv(game->network.spectator_fds[i], peek_buf, 1, MSG_PEEK | MSG_DONTWAIT);

            if (peek_result == 0)
            {
                // 연결 끊김 감지 - MSG_SPECTATOR_LEAVE 브로드캐스트
                char spectator_name[MAX_PLAYER_NAME];
                strncpy(spectator_name, game->network.spectator_names[i], MAX_PLAYER_NAME);

                // 관전자 제거 (spectator_count 감소)
                network_server_remove_spectator(&game->network, i);

                // 퇴장 메시지 전송
                Message leave_msg;
                protocol_init_message(&leave_msg, MSG_SPECTATOR_LEAVE, game->network.sequence_number++);
                strncpy(leave_msg.payload.spectator_join_leave.spectator_name, spectator_name, MAX_PLAYER_NAME);
                leave_msg.payload.spectator_join_leave.spectator_count = game->network.spectator_count;

                // 클라이언트에게 전송
                network_send_message(&game->network, &leave_msg);

                // 다른 관전자들에게 전송
                for (int j = 0; j < MAX_SPECTATORS; j++)
                {
                    if (game->network.spectator_fds[j] >= 0)
                    {
                        network_send_to_spectator(&game->network, j, &leave_msg);
                    }
                }
            }
        }
    }
}

// ============================================================================
// PING 처리
// ============================================================================

#include <sys/time.h>

static uint64_t get_current_time_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
}

void mp_send_ping_if_needed(MultiplayerGame *game)
{
    uint64_t now = get_current_time_ms();

    // 1초마다 PING 전송 (대기 중이 아니면)
    if (!game->network.ping_pending && (now - game->network.ping_last_sent) >= 1000)
    {
        Message ping_msg;
        protocol_init_message(&ping_msg, MSG_PING, game->network.sequence_number++);
        ping_msg.payload.ping_pong.timestamp = now;

        if (mp_send_with_error_check(game, &ping_msg, "sending ping"))
        {
            game->network.ping_last_sent = now;
            game->network.ping_pending = true;
        }
    }
}
