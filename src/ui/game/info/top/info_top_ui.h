#ifndef INFO_TOP_UI_H
#define INFO_TOP_UI_H

#include <ncurses.h>
#include "game/core/stone.h"

// 상단 Info 영역 텍스트 표시 함수 (stdscr에 직접 그림)
void info_top_opponent(const char *name);                    // 상대방 이름 표시
void info_top_opponent_color(const char *name, Stone color); // 상대방 이름 + 색 표시
void info_top_viewers(int count);                            // 뷰어 수 표시
void info_top_ping(int ping_ms);                             // PING 표시
void info_top_port(int port);                                // PORT 표시

#endif // INFO_TOP_UI_H
