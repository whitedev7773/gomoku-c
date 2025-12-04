#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

#include "../../network/network.h"
#include "../core/board.h"

/**
 * 멀티플레이 게임 실행 - 호스트 모드
 * @param port 서버 포트 (0이면 기본 포트 사용)
 * @param rule 게임 규칙 (RULE_STANDARD 또는 RULE_RENJU)
 * @return 게임 종료 시 0, 오류 시 -1
 */
int multiplayer_run_host(int port, GameRule rule);

/**
 * 멀티플레이 게임 실행 - 클라이언트 모드
 * @param server_ip 서버 IP 주소
 * @param port 서버 포트
 * @param rule 게임 규칙 (호스트로부터 받을 예정, 현재는 미사용)
 * @return 게임 종료 시 0, 오류 시 -1
 */
int multiplayer_run_client(const char *server_ip, int port, GameRule rule);

#endif // MULTIPLAYER_H
