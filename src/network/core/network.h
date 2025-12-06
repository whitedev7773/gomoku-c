#ifndef NETWORK_H
#define NETWORK_H

#include "protocol.h"
#include <stdbool.h>
#include <netinet/in.h>

#define DEFAULT_PORT 7773
#define MAX_CONNECTIONS 1
#define MAX_SPECTATORS 3
#define RECV_BUFFER_SIZE 4096

// 네트워크 역할
typedef enum
{
    NETWORK_SERVER,
    NETWORK_CLIENT,
    NETWORK_SPECTATOR
} NetworkRole;

// 네트워크 상태
typedef enum
{
    NET_DISCONNECTED,
    NET_CONNECTING,
    NET_CONNECTED,
    NET_ERROR
} NetworkState;

// 네트워크 매니저
typedef struct
{
    NetworkRole role;
    NetworkState state;

    int socket_fd; // 서버 소켓 (SERVER) or 클라이언트 소켓 (CLIENT/SPECTATOR)
    int client_fd; // 연결된 클라이언트 (SERVER only)
    struct sockaddr_in address;

    char local_ip[16];
    int port;

    char remote_ip[16];
    int remote_port;

    uint32_t sequence_number;

    uint8_t recv_buffer[RECV_BUFFER_SIZE];
    size_t recv_buffer_used;

    // 관전자 관리 (SERVER only)
    int spectator_fds[MAX_SPECTATORS];                     // 관전자 소켓 파일 디스크립터
    char spectator_names[MAX_SPECTATORS][MAX_PLAYER_NAME]; // 관전자 이름
    int spectator_count;                                   // 현재 관전자 수

    // 통계
    uint64_t bytes_sent;
    uint64_t bytes_received;
    uint32_t messages_sent;
    uint32_t messages_received;
} NetworkManager;

// 초기화 및 정리
bool network_init_server(NetworkManager *net, int port);
bool network_init_client(NetworkManager *net);
void network_cleanup(NetworkManager *net);

// 서버 함수
bool network_server_start_listen(NetworkManager *net);
bool network_server_accept_client(NetworkManager *net);
void network_get_local_ip(char *ip_buffer, size_t buffer_size);

// 클라이언트 함수
bool network_client_connect(NetworkManager *net, const char *server_ip, int port);

// 관전자 함수
bool network_init_spectator(NetworkManager *net);
bool network_spectator_connect(NetworkManager *net, const char *server_ip, int port);
int network_server_accept_spectator(NetworkManager *net); // Returns slot index or -1 on failure
void network_server_remove_spectator(NetworkManager *net, int index);
int network_broadcast_to_spectators(NetworkManager *net, const Message *msg);
int network_send_to_spectator(NetworkManager *net, int spectator_index, const Message *msg);

// 송수신
int network_send_message(NetworkManager *net, const Message *msg);
int network_receive_message(NetworkManager *net, Message *msg);

// 유틸리티
bool network_is_connected(const NetworkManager *net);
void network_set_nonblocking(int socket_fd, bool nonblocking);
int network_get_ping_ms(NetworkManager *net);

#endif // NETWORK_H
