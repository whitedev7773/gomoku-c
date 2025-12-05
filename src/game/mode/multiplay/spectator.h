#ifndef SPECTATOR_H
#define SPECTATOR_H

#include "../../../network/network.h"
#include "../../core/board.h"

// 관전자 모드로 게임 관전
// server_ip: 서버 IP 주소
// port: 서버 포트 (기본 7773)
// spectator_name: 관전자 이름 (최대 8자)
// 반환값: 종료 코드 (0: 정상 종료)
int spectator_run(const char *server_ip, int port, const char *spectator_name);

#endif // SPECTATOR_H
