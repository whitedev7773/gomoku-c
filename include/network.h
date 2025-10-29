#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    NET_EVENT_NONE = 0,
    NET_EVENT_CONNECTED,
    NET_EVENT_DISCONNECTED,
    NET_EVENT_MOVE,
    NET_EVENT_CHAT,
    NET_EVENT_UNDO_REQUEST,
    NET_EVENT_UNDO_RESPONSE,
    NET_EVENT_RESIGN,
    NET_EVENT_ERROR
} network_event_type_t;

typedef struct {
    network_event_type_t type;
    union {
        struct {
            char nickname[32];
        } connected;
        struct {
            int row;
            int col;
        } move;
        struct {
            bool accepted;
        } undo;
        struct {
            char message[256];
        } chat;
        struct {
            int code;
        } error;
    } data;
} network_event_t;

typedef void (*network_event_callback)(const network_event_t *event, void *userdata);

typedef struct network_session network_session_t;

int network_start_server(network_session_t **out_session, uint16_t port, const char *nickname,
                         network_event_callback callback, void *userdata);
int network_connect(network_session_t **out_session, const char *host, uint16_t port, const char *nickname,
                    network_event_callback callback, void *userdata);
void network_close(network_session_t *session);

int network_send_move(network_session_t *session, int row, int col);
int network_send_chat(network_session_t *session, const char *message);
int network_send_undo_request(network_session_t *session);
int network_send_undo_response(network_session_t *session, bool accepted);
int network_send_resign(network_session_t *session);

#endif // NETWORK_H
