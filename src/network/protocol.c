#include "protocol.h"
#include <string.h>
#include <arpa/inet.h>

void protocol_init_message(Message *msg, MessageType type, uint32_t sequence) {
    memset(msg, 0, sizeof(Message));
    msg->header.version = PROTOCOL_VERSION;
    msg->header.type = type;
    msg->header.sequence = sequence;
    msg->header.length = protocol_get_message_size(type) - sizeof(MessageHeader);
}

size_t protocol_get_message_size(MessageType type) {
    size_t header_size = sizeof(MessageHeader);

    switch (type) {
        case MSG_CONNECT:
            return header_size + sizeof(ConnectMessage);
        case MSG_CONNECT_ACK:
            return header_size + sizeof(ConnectAckMessage);
        case MSG_PLAYER_INFO:
            return header_size + sizeof(PlayerInfoMessage);
        case MSG_GAME_START:
            return header_size + sizeof(GameStartMessage);
        case MSG_MOVE:
            return header_size + sizeof(MoveMessage);
        case MSG_MOVE_ACK:
            return header_size + sizeof(MoveAckMessage);
        case MSG_CURSOR_UPDATE:
            return header_size + sizeof(CursorUpdateMessage);
        case MSG_CHAT:
            return header_size + sizeof(ChatMessage);
        case MSG_COMMAND:
            return header_size + sizeof(CommandMessage);
        case MSG_COMMAND_RESPONSE:
            return header_size + sizeof(CommandResponseMessage);
        case MSG_PING:
        case MSG_PONG:
            return header_size + sizeof(PingPongMessage);
        case MSG_DISCONNECT:
            return header_size;
        case MSG_ERROR:
            return header_size + sizeof(ErrorMessage);
        case MSG_SPECTATOR_CONNECT:
            return header_size + sizeof(SpectatorConnectMessage);
        case MSG_SPECTATOR_CONNECT_ACK:
            return header_size + sizeof(SpectatorConnectAckMessage);
        case MSG_SPECTATOR_JOIN:
        case MSG_SPECTATOR_LEAVE:
            return header_size + sizeof(SpectatorJoinLeaveMessage);
        case MSG_GAME_STATE:
            return header_size + sizeof(GameStateMessage);
        default:
            return header_size;
    }
}

int protocol_serialize(const Message *msg, uint8_t *buffer, size_t buffer_size) {
    size_t msg_size = protocol_get_message_size(msg->header.type);

    if (buffer_size < msg_size) {
        return -1;  // Buffer too small
    }

    // 헤더 직렬화 (네트워크 바이트 오더)
    uint8_t *ptr = buffer;
    *ptr++ = msg->header.version;
    *ptr++ = msg->header.type;
    uint16_t length_net = htons(msg->header.length);
    memcpy(ptr, &length_net, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    uint32_t seq_net = htonl(msg->header.sequence);
    memcpy(ptr, &seq_net, sizeof(uint32_t));
    ptr += sizeof(uint32_t);

    // 페이로드 직렬화
    size_t payload_size = msg_size - sizeof(MessageHeader);
    if (payload_size > 0) {
        memcpy(ptr, &msg->payload, payload_size);
    }

    return msg_size;
}

int protocol_deserialize(Message *msg, const uint8_t *buffer, size_t buffer_size) {
    if (buffer_size < sizeof(MessageHeader)) {
        return -1;  // Buffer too small for header
    }

    // 헤더 역직렬화
    const uint8_t *ptr = buffer;
    msg->header.version = *ptr++;
    msg->header.type = *ptr++;
    uint16_t length_net;
    memcpy(&length_net, ptr, sizeof(uint16_t));
    msg->header.length = ntohs(length_net);
    ptr += sizeof(uint16_t);
    uint32_t seq_net;
    memcpy(&seq_net, ptr, sizeof(uint32_t));
    msg->header.sequence = ntohl(seq_net);
    ptr += sizeof(uint32_t);

    // 버전 체크
    if (msg->header.version != PROTOCOL_VERSION) {
        return -2;  // Version mismatch
    }

    // 페이로드 역직렬화
    size_t expected_size = protocol_get_message_size(msg->header.type);
    if (buffer_size < expected_size) {
        return -1;  // Buffer too small for payload
    }

    size_t payload_size = msg->header.length;
    if (payload_size > 0) {
        memcpy(&msg->payload, ptr, payload_size);
    }

    return expected_size;
}
