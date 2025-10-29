#include "packet.h"

#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
    uint8_t type;
    uint16_t length;
} __attribute__((packed)) packet_header_t;

static int send_all(int fd, const void *buffer, size_t length) {
    const uint8_t *data = (const uint8_t *)buffer;
    size_t total_sent = 0;
    while (total_sent < length) {
        ssize_t sent = send(fd, data + total_sent, length - total_sent, 0);
        if (sent <= 0) {
            return -1;
        }
        total_sent += (size_t)sent;
    }
    return 0;
}

int packet_send(int fd, packet_type_t type, const void *data, uint16_t length) {
    packet_header_t header;
    header.type = (uint8_t)type;
    header.length = htons(length);
    if (send_all(fd, &header, sizeof(header)) < 0) {
        return -1;
    }
    if (length > 0 && data) {
        if (send_all(fd, data, length) < 0) {
            return -1;
        }
    }
    return 0;
}

int packet_recv(int fd, packet_type_t *type, void *buffer, uint16_t buffer_size, uint16_t *out_length) {
    packet_header_t header;
    ssize_t received = recv(fd, &header, sizeof(header), MSG_WAITALL);
    if (received == 0) {
        return -1; // connection closed
    }
    if (received < 0) {
        return -1;
    }
    if ((size_t)received != sizeof(header)) {
        return -1;
    }
    uint16_t payload_length = ntohs(header.length);
    if (payload_length > buffer_size) {
        // consume and discard
        uint8_t discard[512];
        size_t remaining = payload_length;
        while (remaining > 0) {
            size_t chunk = remaining > sizeof(discard) ? sizeof(discard) : remaining;
            ssize_t r = recv(fd, discard, chunk, MSG_WAITALL);
            if (r <= 0) {
                return -1;
            }
            remaining -= (size_t)r;
        }
        return -1;
    }
    if (payload_length > 0) {
        ssize_t payload_received = recv(fd, buffer, payload_length, MSG_WAITALL);
        if (payload_received <= 0 || (uint16_t)payload_received != payload_length) {
            return -1;
        }
    }
    if (type) {
        *type = (packet_type_t)header.type;
    }
    if (out_length) {
        *out_length = payload_length;
    }
    return 0;
}
