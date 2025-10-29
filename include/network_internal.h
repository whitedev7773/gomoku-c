#ifndef NETWORK_INTERNAL_H
#define NETWORK_INTERNAL_H

#include <pthread.h>

#include "network.h"
#include "packet.h"

struct network_session {
    int listen_fd;
    int socket_fd;
    pthread_t thread;
    int running;
    int is_server;
    int thread_started;
    network_event_callback callback;
    void *userdata;
    pthread_mutex_t send_lock;
    char local_nickname[32];
    char peer_nickname[32];
};

int network_session_init(network_session_t *session, const char *nickname, network_event_callback cb, void *userdata, int is_server);
void network_session_emit(network_session_t *session, const network_event_t *event);
void network_session_close_socket(network_session_t *session);
void network_session_destroy(network_session_t *session);
int network_session_handle_packet(network_session_t *session, packet_type_t type, const void *payload, uint16_t length);

#endif // NETWORK_INTERNAL_H
