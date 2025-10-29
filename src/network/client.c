#include "network_internal.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define NETWORK_BUFFER_SIZE 512

static void network_emit_error(network_session_t *session, int code) {
    if (!session) {
        return;
    }
    network_event_t event;
    memset(&event, 0, sizeof(event));
    event.type = NET_EVENT_ERROR;
    event.data.error.code = code;
    network_session_emit(session, &event);
}

static void network_emit_disconnect(network_session_t *session) {
    network_event_t event;
    memset(&event, 0, sizeof(event));
    event.type = NET_EVENT_DISCONNECTED;
    network_session_emit(session, &event);
}

static void network_emit_connected(network_session_t *session) {
    network_event_t event;
    memset(&event, 0, sizeof(event));
    event.type = NET_EVENT_CONNECTED;
    strncpy(event.data.connected.nickname, session->peer_nickname,
            sizeof(event.data.connected.nickname) - 1);
    event.data.connected.nickname[sizeof(event.data.connected.nickname) - 1] = '\0';
    network_session_emit(session, &event);
}

static int network_send_handshake(network_session_t *session) {
    size_t len = strnlen(session->local_nickname, sizeof(session->local_nickname));
    return packet_send(session->socket_fd, PACKET_HELLO, session->local_nickname, (uint16_t)(len + 1));
}

static int network_receive_handshake(network_session_t *session) {
    uint8_t buffer[NETWORK_BUFFER_SIZE];
    uint16_t length = 0;
    packet_type_t type;
    if (packet_recv(session->socket_fd, &type, buffer, sizeof(buffer), &length) < 0) {
        return -1;
    }
    if (type != PACKET_HELLO) {
        return -1;
    }
    size_t copy_len = length < sizeof(session->peer_nickname) - 1 ?
                      length : sizeof(session->peer_nickname) - 1;
    memcpy(session->peer_nickname, buffer, copy_len);
    session->peer_nickname[copy_len] = '\0';
    return 0;
}

static void *client_thread(void *arg) {
    network_session_t *session = (network_session_t *)arg;
    if (!session) {
        return NULL;
    }
    uint8_t buffer[NETWORK_BUFFER_SIZE];
    while (session->running) {
        uint16_t length = 0;
        packet_type_t type;
        int res = packet_recv(session->socket_fd, &type, buffer, sizeof(buffer), &length);
        if (res < 0) {
            break;
        }
        if (network_session_handle_packet(session, type, buffer, length) < 0) {
            break;
        }
    }
    network_emit_disconnect(session);
    network_session_close_socket(session);
    session->running = 0;
    return NULL;
}

int network_connect(network_session_t **out_session, const char *host, uint16_t port, const char *nickname,
                    network_event_callback callback, void *userdata) {
    if (!out_session || !host) {
        return -1;
    }
    network_session_t *session = calloc(1, sizeof(network_session_t));
    if (!session) {
        return -1;
    }
    if (network_session_init(session, nickname, callback, userdata, 0) < 0) {
        free(session);
        return -1;
    }

    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%u", port);
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *result = NULL;
    int gai = getaddrinfo(host, port_str, &hints, &result);
    if (gai != 0) {
        network_emit_error(session, gai);
        network_session_destroy(session);
        free(session);
        return -1;
    }

    int connected = 0;
    for (struct addrinfo *rp = result; rp != NULL; rp = rp->ai_next) {
        session->socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (session->socket_fd < 0) {
            continue;
        }
        if (connect(session->socket_fd, rp->ai_addr, rp->ai_addrlen) == 0) {
            connected = 1;
            break;
        }
        close(session->socket_fd);
        session->socket_fd = -1;
    }
    freeaddrinfo(result);

    if (!connected) {
        network_emit_error(session, errno);
        network_session_destroy(session);
        free(session);
        return -1;
    }

    if (network_send_handshake(session) < 0) {
        network_emit_error(session, -5);
        network_session_destroy(session);
        free(session);
        return -1;
    }
    if (network_receive_handshake(session) < 0) {
        network_emit_error(session, -6);
        network_session_destroy(session);
        free(session);
        return -1;
    }

    session->running = 1;
    network_emit_connected(session);

    if (pthread_create(&session->thread, NULL, client_thread, session) != 0) {
        network_session_destroy(session);
        free(session);
        return -1;
    }
    session->thread_started = 1;

    *out_session = session;
    return 0;
}
