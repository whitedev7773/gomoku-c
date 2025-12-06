#define _XOPEN_SOURCE_EXTENDED

#include "ingame_border.h"
#include "../../core/ui_manager.h"
#include "../../core/theme.h"
#include <wchar.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

// 인게임 전체 Border를 stdscr에 그림
// 레이아웃 참조: example-ui/ingame.txt

// 방향 비트 정의
#define UP 1    // 0001
#define DOWN 2  // 0010
#define LEFT 4  // 0100
#define RIGHT 8 // 1000

// 문자에 해당하는 비트마스크를 반환하는 함수
int get_mask_from_char(const char *str)
{
    // 3바이트 UTF-8 문자열 비교 (환경에 따라 수정 가능)
    // 넓은 문자(wchar_t)를 사용하는 것이 더 정확하지만,
    // 여기서는 사용자가 제공한 문자열 형식을 따릅니다.

    // 코너
    if (strcmp(str, "┏") == 0)
        return DOWN | RIGHT;
    if (strcmp(str, "┓") == 0)
        return DOWN | LEFT;
    if (strcmp(str, "┗") == 0)
        return UP | RIGHT;
    if (strcmp(str, "┛") == 0)
        return UP | LEFT;

    // 직선
    if (strcmp(str, "━") == 0)
        return LEFT | RIGHT;
    if (strcmp(str, "┃") == 0)
        return UP | DOWN;

    // T자형 및 십자
    if (strcmp(str, "┣") == 0)
        return UP | DOWN | RIGHT;
    if (strcmp(str, "┫") == 0)
        return UP | DOWN | LEFT;
    if (strcmp(str, "┳") == 0)
        return LEFT | RIGHT | DOWN;
    if (strcmp(str, "┻") == 0)
        return LEFT | RIGHT | UP;
    if (strcmp(str, "╋") == 0)
        return UP | DOWN | LEFT | RIGHT;

    return 0; // 박스 문자가 아닌 경우
}

// 비트마스크에 해당하는 문자를 반환하는 함수
const char *get_char_from_mask(int mask)
{
    switch (mask)
    {
    case LEFT | RIGHT:
        return "━";
    case UP | DOWN:
        return "┃";
    case DOWN | RIGHT:
        return "┏";
    case DOWN | LEFT:
        return "┓";
    case UP | RIGHT:
        return "┗";
    case UP | LEFT:
        return "┛";

    case UP | DOWN | RIGHT:
        return "┣";
    case UP | DOWN | LEFT:
        return "┫";
    case LEFT | RIGHT | DOWN:
        return "┳";
    case LEFT | RIGHT | UP:
        return "┻";

    case UP | DOWN | LEFT | RIGHT:
        return "╋";

    default:
        return " "; // 매칭되지 않는 경우
    }
}

void smart_box_draw(WINDOW *win, int y, int x, const char *new_char_str)
{
    cchar_t existing_cell;
    char existing_str[10] = {0};

    // 1. 현재 위치의 문자를 읽어옴
    if (mvwin_wch(win, y, x, &existing_cell) != OK)
    {
        // 읽기 실패 시 그냥 덮어쓰기
        mvwprintw(win, y, x, "%s", new_char_str);
        return;
    }

    // cchar_t에서 문자열 추출 (ncursesw 사용 시)
    wchar_t wch[2];
    attr_t attrs;
    short color_pair;
    getcchar(&existing_cell, wch, &attrs, &color_pair, NULL);
    wcstombs(existing_str, wch, sizeof(existing_str));

    // 2. 마스크 계산 (기존 문자 + 새 문자)
    int old_mask = get_mask_from_char(existing_str);
    int new_mask = get_mask_from_char(new_char_str);

    // 3. 두 마스크를 합침 (OR 연산)
    // 예: "━"(좌우) 위에 "┃"(상하)를 그리면 -> 좌우상하("╋")가 됨
    int final_mask = old_mask | new_mask;

    // 4. 합쳐진 마스크에 해당하는 문자로 출력
    // 만약 기존에 아무것도 없었다면(0), 새 문자 그대로 출력됨
    if (final_mask > 0)
    {
        mvwprintw(win, y, x, "%s", get_char_from_mask(final_mask));
    }
    else
    {
        mvwprintw(win, y, x, "%s", new_char_str);
    }
}

// 박스 먼저 그리기 (박스를 그리고 세부 조정하는 방향으로)
void ingame_draw_box(const int start_x, const int start_y,
                     const int width, const int height)
{
    // 테마 주색상으로 border 그리기
    wattron(stdscr, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 상단 (Top)
    smart_box_draw(stdscr, start_y, start_x, "┏");
    for (int i = 1; i < width - 1; i++)
    {
        smart_box_draw(stdscr, start_y, start_x + i, "━");
    }
    smart_box_draw(stdscr, start_y, start_x + width - 1, "┓");

    // 중단 (Middle)
    for (int j = 1; j < height - 1; j++)
    {
        smart_box_draw(stdscr, start_y + j, start_x, "┃");
        smart_box_draw(stdscr, start_y + j, start_x + width - 1, "┃");
    }

    // 하단 (Bottom)
    smart_box_draw(stdscr, start_y + height - 1, start_x, "┗");
    for (int i = 1; i < width - 1; i++)
    {
        smart_box_draw(stdscr, start_y + height - 1, start_x + i, "━");
    }
    smart_box_draw(stdscr, start_y + height - 1, start_x + width - 1, "┛");

    wattroff(stdscr, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
}

// 100*30
void ingame_border_draw(void)
{
    // 여기에 전부 그리면 됨
    ingame_border_redraw_board_area();  // 오목판이 표시될 공간
    ingame_border_redraw_info_area();   // 상단에 게임 정보가 표시될 공간 (상대방 이름, N of Viewers, PING nnms, PORT nnnn)
    ingame_border_redraw_chat_area();   // 우측 채팅 공간
    ingame_border_redraw_bottom_area(); // 하단 게임 정보 공간 (이전에 둔 위치, 현재 차례(P1, P2 / ME, AI), Time Progress, Play Time, System Log)
}

// Board 영역 테두리만 재그리기
void ingame_border_redraw_board_area(void)
{
    ingame_draw_box(0, 0, 49, 25); // 보드 영역
}

// 상단 Info 영역 테두리만 재그리기
void ingame_border_redraw_info_area(void)
{
    ingame_draw_box(48, 0, 12, 3); // 플레이어 이름
    ingame_draw_box(59, 0, 16, 3); // N of Viewers
    ingame_draw_box(74, 0, 14, 3); // PING
    ingame_draw_box(87, 0, 13, 3); // PORT
}

// Chat 영역 테두리만 재그리기
void ingame_border_redraw_chat_area(void)
{
    ingame_draw_box(48, 2, 52, 21); // CHAT AREA
    ingame_draw_box(48, 22, 52, 3); // INPUT AREA
    ingame_draw_box(92, 22, 8, 3);  // SEND
}

// Bottom 영역 테두리만 재그리기
void ingame_border_redraw_bottom_area(void)
{
    ingame_draw_box(0, 24, 19, 3);  // LAST MOVE - TOP
    ingame_draw_box(0, 24, 19, 7);  // LAST MOVE - BOTTOM
    ingame_draw_box(18, 24, 15, 3); // NOW TURN - TOP
    ingame_draw_box(18, 24, 15, 7); // NOW TURN - BOTTOM
    ingame_draw_box(32, 24, 48, 3); // TIMER
    ingame_draw_box(79, 24, 21, 3); // PLAY TIME
    ingame_draw_box(32, 26, 68, 5); // SYSTEM LOG
}

// 전체 Border 재그리기
void ingame_border_redraw_all(void)
{
    ingame_border_draw();
}
