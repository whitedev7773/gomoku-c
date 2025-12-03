#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

#include "../network/network.h"
#include "board.h"

/**
 * 멀티플레이 게임 실행 - 호스트 모드
 * @param port 서버 포트 (0이면 기본 포트 사용)
 * @return 게임 종료 시 0, 오류 시 -1
 */
int multiplayer_run_host(int port);

/**
 * 멀티플레이 게임 실행 - 클라이언트 모드
 * @param server_ip 서버 IP 주소
 * @param port 서버 포트
 * @return 게임 종료 시 0, 오류 시 -1
 */
int multiplayer_run_client(const char *server_ip, int port);

#endif // MULTIPLAYER_H
