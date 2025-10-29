#include "network_internal.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
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

int network_session_init(network_session_t *session, const char *nickname, network_event_callback cb,
                         void *userdata, int is_server) {
    if (!session) {
        return -1;
    }
    memset(session, 0, sizeof(*session));
    session->listen_fd = -1;
    session->socket_fd = -1;
    session->running = 0;
    session->is_server = is_server;
    session->callback = cb;
    session->userdata = userdata;
    pthread_mutex_init(&session->send_lock, NULL);
    if (nickname) {
        strncpy(session->local_nickname, nickname, sizeof(session->local_nickname) - 1);
    } else {
        snprintf(session->local_nickname, sizeof(session->local_nickname), "Player");
    }
    session->local_nickname[sizeof(session->local_nickname) - 1] = '\0';
    session->peer_nickname[0] = '\0';
    return 0;
}

void network_session_emit(network_session_t *session, const network_event_t *event) {
    if (!session || !session->callback || !event) {
        return;
    }
    session->callback(event, session->userdata);
}

void network_session_close_socket(network_session_t *session) {
    if (!session) {
        return;
    }
    if (session->socket_fd >= 0) {
        close(session->socket_fd);
        session->socket_fd = -1;
    }
    if (session->listen_fd >= 0) {
        close(session->listen_fd);
        session->listen_fd = -1;
    }
}

void network_session_destroy(network_session_t *session) {
    if (!session) {
        return;
    }
    network_session_close_socket(session);
    pthread_mutex_destroy(&session->send_lock);
}

int network_session_handle_packet(network_session_t *session, packet_type_t type, const void *payload, uint16_t length) {
    if (!session) {
        return -1;
    }
    network_event_t event;
    memset(&event, 0, sizeof(event));
    switch (type) {
        case PACKET_MOVE:
            if (length < 2) {
                return -1;
            }
            event.type = NET_EVENT_MOVE;
            event.data.move.row = ((const uint8_t *)payload)[0];
            event.data.move.col = ((const uint8_t *)payload)[1];
            network_session_emit(session, &event);
            break;
        case PACKET_CHAT: {
            event.type = NET_EVENT_CHAT;
            size_t copy_len = length < sizeof(event.data.chat.message) - 1 ?
                              length : sizeof(event.data.chat.message) - 1;
            memcpy(event.data.chat.message, payload, copy_len);
            event.data.chat.message[copy_len] = '\0';
            network_session_emit(session, &event);
            break;
        }
        case PACKET_UNDO_REQUEST:
            event.type = NET_EVENT_UNDO_REQUEST;
            network_session_emit(session, &event);
            break;
        case PACKET_UNDO_RESPONSE:
            if (length < 1) {
                return -1;
            }
            event.type = NET_EVENT_UNDO_RESPONSE;
            event.data.undo.accepted = ((const uint8_t *)payload)[0] != 0;
            network_session_emit(session, &event);
            break;
        case PACKET_RESIGN:
            event.type = NET_EVENT_RESIGN;
            network_session_emit(session, &event);
            break;
        default:
            event.type = NET_EVENT_ERROR;
            event.data.error.code = -2;
            network_session_emit(session, &event);
            return -1;
    }
    return 0;
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

static void *server_thread(void *arg) {
    network_session_t *session = (network_session_t *)arg;
    if (!session) {
        return NULL;
    }
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    session->socket_fd = accept(session->listen_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (session->socket_fd < 0) {
        network_emit_error(session, errno);
        session->running = 0;
        return NULL;
    }
    if (session->listen_fd >= 0) {
        close(session->listen_fd);
        session->listen_fd = -1;
    }
    // Handshake: receive client hello then send ours
    if (network_receive_handshake(session) < 0) {
        network_emit_error(session, -3);
        network_session_close_socket(session);
        session->running = 0;
        return NULL;
    }
    if (network_send_handshake(session) < 0) {
        network_emit_error(session, -4);
        network_session_close_socket(session);
        session->running = 0;
        return NULL;
    }

    network_emit_connected(session);

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

int network_start_server(network_session_t **out_session, uint16_t port, const char *nickname,
                         network_event_callback callback, void *userdata) {
    if (!out_session) {
        return -1;
    }
    network_session_t *session = calloc(1, sizeof(network_session_t));
    if (!session) {
        return -1;
    }
    if (network_session_init(session, nickname, callback, userdata, 1) < 0) {
        free(session);
        return -1;
    }

    session->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (session->listen_fd < 0) {
        network_session_destroy(session);
        free(session);
        return -1;
    }

    int opt = 1;
    setsockopt(session->listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(session->listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        network_session_destroy(session);
        free(session);
        return -1;
    }

    if (listen(session->listen_fd, 1) < 0) {
        network_session_destroy(session);
        free(session);
        return -1;
    }

    session->running = 1;
    if (pthread_create(&session->thread, NULL, server_thread, session) != 0) {
        network_session_destroy(session);
        free(session);
        return -1;
    }
    session->thread_started = 1;

    *out_session = session;
    return 0;
}

void network_close(network_session_t *session) {
    if (!session) {
        return;
    }
    session->running = 0;
    network_session_close_socket(session);
    if (session->thread_started) {
        pthread_join(session->thread, NULL);
    }
    network_session_destroy(session);
    free(session);
}

static int network_send_payload(network_session_t *session, packet_type_t type, const void *payload, uint16_t length) {
    if (!session || session->socket_fd < 0) {
        return -1;
    }
    pthread_mutex_lock(&session->send_lock);
    int result = packet_send(session->socket_fd, type, payload, length);
    pthread_mutex_unlock(&session->send_lock);
    return result;
}

int network_send_move(network_session_t *session, int row, int col) {
    uint8_t payload[2];
    payload[0] = (uint8_t)row;
    payload[1] = (uint8_t)col;
    return network_send_payload(session, PACKET_MOVE, payload, sizeof(payload));
}

int network_send_chat(network_session_t *session, const char *message) {
    if (!message) {
        return -1;
    }
    size_t len = strnlen(message, 255);
    uint16_t payload_len = (uint16_t)(len + 1);
    return network_send_payload(session, PACKET_CHAT, message, payload_len);
}

int network_send_undo_request(network_session_t *session) {
    return network_send_payload(session, PACKET_UNDO_REQUEST, NULL, 0);
}

int network_send_undo_response(network_session_t *session, bool accepted) {
    uint8_t payload = accepted ? 1 : 0;
    return network_send_payload(session, PACKET_UNDO_RESPONSE, &payload, 1);
}

int network_send_resign(network_session_t *session) {
    return network_send_payload(session, PACKET_RESIGN, NULL, 0);
}
