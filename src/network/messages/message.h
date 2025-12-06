#ifndef MESSAGE_H
#define MESSAGE_H

// 모든 메시지 타입 헤더 통합 include
#include "message_types.h"
#include "connection.h"
#include "game.h"
#include "chat.h"
#include "spectator.h"

// 전체 메시지 (헤더 + 페이로드)
typedef struct
{
    MessageHeader header;
    union
    {
        // Connection messages
        ConnectMessage connect;
        ConnectAckMessage connect_ack;
        PlayerInfoMessage player_info;
        PingPongMessage ping_pong;
        ErrorMessage error;

        // Game messages
        GameStartMessage game_start;
        MoveMessage move;
        MoveAckMessage move_ack;
        CursorUpdateMessage cursor;
        GameStateMessage game_state;
        GameResultMessage game_result;

        // Chat messages
        ChatMessage chat;
        CommandMessage command;
        CommandResponseMessage command_response;

        // Spectator messages
        SpectatorConnectMessage spectator_connect;
        SpectatorConnectAckMessage spectator_connect_ack;
        SpectatorJoinLeaveMessage spectator_join_leave;
    } payload;
} Message;

#endif // MESSAGE_H
