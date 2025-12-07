// ============================================================================
// mp_input.c - 사용자 입력 처리 (모달, 채팅)
// ============================================================================

#include "mp_input.h"
#include "mp_network.h"
#include <string.h>
#include <ncurses.h>

// ============================================================================
// 모달 입력 처리
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
        // 기권 요청 전송
        if (game->modal_ui.type == MODAL_GIVEUP)
        {
            modal_ui_close(&game->modal_ui);

            // 기권 요청 메시지 전송
            Message msg;
            protocol_init_message(&msg, MSG_COMMAND, game->network.sequence_number++);
            msg.payload.command.command_type = CMD_GIVEUP;
            mp_send_with_error_check(game, &msg, "sending giveup request");

            char modal_msg[MODAL_MAX_MESSAGE_LENGTH];
            snprintf(modal_msg, sizeof(modal_msg), "Waiting for opponent's response...");
            modal_ui_show(&game->modal_ui, MODAL_GIVEUP_REQUEST, modal_msg);
            chat_add_msg(&game->chat_ui, "Giveup request sent", CHAT_MSG_SYSTEM);

            ret = MP_MODAL_CLOSED;
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
        else if (game->modal_ui.type == MODAL_GIVEUP_RESPONSE)
        {
            game->game_over = true;
            game->result = (game->opponent.color == BLACK) ? GAME_WHITE_WIN : GAME_BLACK_WIN;

            Message msg;
            protocol_init_message(&msg, MSG_COMMAND_RESPONSE, game->network.sequence_number++);
            msg.payload.command_response.command_type = CMD_GIVEUP;
            msg.payload.command_response.accepted = 1;
            strncpy(msg.payload.command_response.message, "Giveup accepted", sizeof(msg.payload.command_response.message) - 1);
            network_send_message(&game->network, &msg);

            modal_ui_close(&game->modal_ui);
            char giveup_msg[128];
            snprintf(giveup_msg, sizeof(giveup_msg), "%s gave up. %s wins!",
                     game->opponent.name, game->me.name);
            modal_ui_show(&game->modal_ui, MODAL_GAME_RESULT, giveup_msg);
            chat_add_msg(&game->chat_ui, giveup_msg, CHAT_MSG_SYSTEM);

            // 관전자에게 기권 결과 전송
            uint8_t giveup_result = (game->result == GAME_BLACK_WIN) ? RESULT_BLACK_WIN : RESULT_WHITE_WIN;
            mp_broadcast_game_result_to_spectators(game, giveup_result, REASON_GIVEUP, game->me.name, giveup_msg);

            ret = MP_MODAL_GAME_OVER;
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
        else if (game->modal_ui.type == MODAL_GIVEUP_RESPONSE)
        {
            Message msg;
            protocol_init_message(&msg, MSG_COMMAND_RESPONSE, game->network.sequence_number++);
            msg.payload.command_response.command_type = CMD_GIVEUP;
            msg.payload.command_response.accepted = 0;
            strncpy(msg.payload.command_response.message, "Giveup declined", sizeof(msg.payload.command_response.message) - 1);
            network_send_message(&game->network, &msg);
            chat_add_msg(&game->chat_ui, "Giveup declined", CHAT_MSG_SYSTEM);
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

                // 게임 경과 시간 계산 (초 단위)
                int game_time_seconds = (int)difftime(time(NULL), game->game_start_time);
                chat_add_msg_with_time(&game->chat_ui, msg_text, CHAT_MSG_USER, game_time_seconds);
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
