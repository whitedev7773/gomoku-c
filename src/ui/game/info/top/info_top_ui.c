#include "info_top_ui.h"
#include <stdio.h>
#include <string.h>

// ============================================
// 상단 Info 영역 텍스트 표시 함수 (stdscr에 직접 그림)
// ============================================

// 상대방 이름 표시 - 위치 (49, 1)
void info_top_opponent(const char *name)
{
    // 영역 클리어 (10칸)
    move(1, 49);
    for (int i = 0; i < 10; i++)
        addch(' ');
    // 이름 표시
    mvaddstr(1, 50, name);
    refresh();
}

// 상대방 이름 + 색 표시 - 위치 (49, 1)
void info_top_opponent_color(const char *name, Stone color)
{
    // 영역 클리어 (15칸)
    move(1, 49);
    for (int i = 0; i < 15; i++)
        addch(' ');
    // 이름만 표시
    mvaddstr(1, 50, name);
    refresh();
}

// 뷰어 수 표시 - 위치 (60, 1) - 형식: "{n} of Viewers"
void info_top_viewers(int count)
{
    // 영역 클리어 (14칸)
    move(1, 60);
    for (int i = 0; i < 14; i++)
        addch(' ');
    // 뷰어 수 표시
    char buf[32];
    snprintf(buf, sizeof(buf), "%d of Viewers", count);
    mvaddstr(1, 61, buf);
    refresh();
}

// PING 표시 - 위치 (75, 1) - 형식: "PING {nn}ms"
void info_top_ping(int ping_ms)
{
    // 영역 클리어 (12칸)
    move(1, 75);
    for (int i = 0; i < 12; i++)
        addch(' ');
    // PING 표시
    char buf[32];
    snprintf(buf, sizeof(buf), "PING %dms", ping_ms);
    mvaddstr(1, 76, buf);
    refresh();
}

// PORT 표시 - 위치 (88, 1) - 형식: "PORT {nnnn}"
void info_top_port(int port)
{
    // 영역 클리어 (11칸)
    move(1, 88);
    for (int i = 0; i < 11; i++)
        addch(' ');
    // PORT 표시
    char buf[32];
    snprintf(buf, sizeof(buf), "PORT %d", port);
    mvaddstr(1, 89, buf);
    refresh();
}
