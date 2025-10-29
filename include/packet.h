#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

typedef enum {
    PACKET_HELLO = 1,
    PACKET_MOVE = 2,
    PACKET_CHAT = 3,
    PACKET_UNDO_REQUEST = 4,
    PACKET_UNDO_RESPONSE = 5,
    PACKET_RESIGN = 6
} packet_type_t;

int packet_send(int fd, packet_type_t type, const void *data, uint16_t length);
int packet_recv(int fd, packet_type_t *type, void *buffer, uint16_t buffer_size, uint16_t *out_length);

#endif // PACKET_H
