#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <ifaddrs.h>

bool network_init_server(NetworkManager *net, int port) {
    memset(net, 0, sizeof(NetworkManager));
    net->role = NETWORK_SERVER;
    net->state = NET_DISCONNECTED;
    net->port = port;
    net->socket_fd = -1;
    net->client_fd = -1;

    // 소켓 생성
    net->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (net->socket_fd < 0) {
        perror("socket");
        return false;
    }

    // SO_REUSEADDR 설정
    int opt = 1;
    if (setsockopt(net->socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(net->socket_fd);
        return false;
    }

    // TCP_NODELAY 설정 (Nagle 알고리즘 비활성화)
    if (setsockopt(net->socket_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0) {
        perror("setsockopt TCP_NODELAY");
    }

    // 주소 설정
    memset(&net->address, 0, sizeof(net->address));
    net->address.sin_family = AF_INET;
    net->address.sin_addr.s_addr = INADDR_ANY;
    net->address.sin_port = htons(port);

    // 바인드
    if (bind(net->socket_fd, (struct sockaddr *)&net->address, sizeof(net->address)) < 0) {
        perror("bind");
        close(net->socket_fd);
        return false;
    }

    // 로컬 IP 가져오기
    network_get_local_ip(net->local_ip, sizeof(net->local_ip));

    return true;
}

bool network_server_start_listen(NetworkManager *net) {
    if (net->role != NETWORK_SERVER || net->socket_fd < 0) {
        return false;
    }

    if (listen(net->socket_fd, MAX_CONNECTIONS) < 0) {
        perror("listen");
        return false;
    }

    net->state = NET_CONNECTING;
    return true;
}

bool network_server_accept_client(NetworkManager *net) {
    if (net->role != NETWORK_SERVER || net->socket_fd < 0) {
        return false;
    }

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    net->client_fd = accept(net->socket_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (net->client_fd < 0) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            perror("accept");
        }
        return false;
    }

    // 클라이언트 정보 저장
    inet_ntop(AF_INET, &client_addr.sin_addr, net->remote_ip, sizeof(net->remote_ip));
    net->remote_port = ntohs(client_addr.sin_port);

    // TCP_NODELAY 설정
    int opt = 1;
    setsockopt(net->client_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));

    net->state = NET_CONNECTED;
    return true;
}

void network_get_local_ip(char *ip_buffer, size_t buffer_size) {
    struct ifaddrs *ifaddr, *ifa;

    strcpy(ip_buffer, "127.0.0.1");  // 기본값

    if (getifaddrs(&ifaddr) == -1) {
        return;
    }

    // 첫 번째 non-loopback IPv4 주소 찾기
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            char *ip_str = inet_ntoa(addr->sin_addr);

            // 루프백이 아닌 주소
            if (strcmp(ip_str, "127.0.0.1") != 0) {
                strncpy(ip_buffer, ip_str, buffer_size - 1);
                ip_buffer[buffer_size - 1] = '\0';
                break;
            }
        }
    }

    freeifaddrs(ifaddr);
}

bool network_init_client(NetworkManager *net) {
    memset(net, 0, sizeof(NetworkManager));
    net->role = NETWORK_CLIENT;
    net->state = NET_DISCONNECTED;
    net->socket_fd = -1;

    return true;
}

bool network_client_connect(NetworkManager *net, const char *server_ip, int port) {
    if (net->role != NETWORK_CLIENT) {
        return false;
    }

    // 소켓 생성
    net->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (net->socket_fd < 0) {
        perror("socket");
        return false;
    }

    // TCP_NODELAY 설정
    int opt = 1;
    setsockopt(net->socket_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));

    // 서버 주소 설정
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(net->socket_fd);
        net->socket_fd = -1;
        return false;
    }

    // 연결
    net->state = NET_CONNECTING;
    if (connect(net->socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(net->socket_fd);
        net->socket_fd = -1;
        net->state = NET_ERROR;
        return false;
    }

    strncpy(net->remote_ip, server_ip, sizeof(net->remote_ip) - 1);
    net->remote_port = port;
    net->state = NET_CONNECTED;

    return true;
}

void network_cleanup(NetworkManager *net) {
    if (net->client_fd >= 0) {
        close(net->client_fd);
        net->client_fd = -1;
    }

    if (net->socket_fd >= 0) {
        close(net->socket_fd);
        net->socket_fd = -1;
    }

    net->state = NET_DISCONNECTED;
}

int network_send_message(NetworkManager *net, const Message *msg) {
    if (!network_is_connected(net)) {
        return -1;
    }

    uint8_t buffer[RECV_BUFFER_SIZE];
    int size = protocol_serialize(msg, buffer, sizeof(buffer));
    if (size < 0) {
        return -1;
    }

    int fd = (net->role == NETWORK_SERVER) ? net->client_fd : net->socket_fd;
    ssize_t sent = send(fd, buffer, size, 0);

    if (sent < 0) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            perror("send");
            net->state = NET_ERROR;
        }
        return -1;
    }

    net->bytes_sent += sent;
    net->messages_sent++;

    return sent;
}

int network_receive_message(NetworkManager *net, Message *msg) {
    if (!network_is_connected(net)) {
        return -1;
    }

    int fd = (net->role == NETWORK_SERVER) ? net->client_fd : net->socket_fd;

    // 헤더만큼 먼저 읽기
    if (net->recv_buffer_used < sizeof(MessageHeader)) {
        ssize_t received = recv(fd, net->recv_buffer + net->recv_buffer_used,
                               sizeof(MessageHeader) - net->recv_buffer_used, MSG_DONTWAIT);

        if (received < 0) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                perror("recv");
                net->state = NET_ERROR;
            }
            return 0;  // No data available
        } else if (received == 0) {
            // Connection closed
            net->state = NET_DISCONNECTED;
            return -1;
        }

        net->recv_buffer_used += received;

        if (net->recv_buffer_used < sizeof(MessageHeader)) {
            return 0;  // Need more data
        }
    }

    // 헤더 파싱하여 전체 메시지 크기 확인
    MessageHeader temp_header;
    memcpy(&temp_header, net->recv_buffer, sizeof(MessageHeader));
    temp_header.length = ntohs(*(uint16_t *)(net->recv_buffer + 2));
    size_t total_size = sizeof(MessageHeader) + temp_header.length;

    // 전체 메시지 읽기
    if (net->recv_buffer_used < total_size) {
        ssize_t received = recv(fd, net->recv_buffer + net->recv_buffer_used,
                               total_size - net->recv_buffer_used, MSG_DONTWAIT);

        if (received < 0) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                perror("recv");
                net->state = NET_ERROR;
            }
            return 0;
        } else if (received == 0) {
            net->state = NET_DISCONNECTED;
            return -1;
        }

        net->recv_buffer_used += received;

        if (net->recv_buffer_used < total_size) {
            return 0;  // Need more data
        }
    }

    // 메시지 역직렬화
    int result = protocol_deserialize(msg, net->recv_buffer, net->recv_buffer_used);
    if (result < 0) {
        // 역직렬화 실패, 버퍼 초기화
        net->recv_buffer_used = 0;
        return -1;
    }

    // 성공, 버퍼에서 메시지 제거
    net->recv_buffer_used -= result;
    if (net->recv_buffer_used > 0) {
        memmove(net->recv_buffer, net->recv_buffer + result, net->recv_buffer_used);
    }

    net->bytes_received += result;
    net->messages_received++;

    return result;
}

bool network_is_connected(const NetworkManager *net) {
    return net->state == NET_CONNECTED;
}

void network_set_nonblocking(int socket_fd, bool nonblocking) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl F_GETFL");
        return;
    }

    if (nonblocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }

    if (fcntl(socket_fd, F_SETFL, flags) < 0) {
        perror("fcntl F_SETFL");
    }
}

int network_get_ping_ms(NetworkManager *net) {
    // TODO: PING/PONG 메시지로 실제 RTT 측정
    return 0;
}
