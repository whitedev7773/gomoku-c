#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include "../messages/message.h"

// 프로토콜 함수
void protocol_init_message(Message *msg, MessageType type, uint32_t sequence);
int protocol_serialize(const Message *msg, uint8_t *buffer, size_t buffer_size);
int protocol_deserialize(Message *msg, const uint8_t *buffer, size_t buffer_size);
size_t protocol_get_message_size(MessageType type);

#endif // PROTOCOL_H
