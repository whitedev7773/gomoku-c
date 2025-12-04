#include "ingame_border.h"
#include "ui_manager.h"
#include "theme.h"

// 인게임 전체 Border를 stdscr에 그림
// 레이아웃 참조: example-ui/ingame.txt
//
// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━┯━━━━━━━━━━━━━━┯━━━━━━━━━━━━┯━━━━━━━━━━━┓
// ┃     (Board Area - 51 cols)                    ┃ (Info Area - 49 cols)                          ┃
// ┃                                               ┣━━━━━━━━━━┷━━━━━━━━━━━━━━┷━━━━━━━━━━━━┷━━━━━━━━━━━┫
// ┃                                               ┃                                                  ┃
// ┃                                               ┃  (Chat Area - 21 rows)                          ┃
// ┃                                               ┃                                                  ┃
// ┃                                               ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┯━━━━━━━━━━┫
// ┃                                               ┃ (Chat Input Area - 3 rows)           │          ┃
// ┣━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━┯━━━━━━━━━━┷━━━━━━━━━━┫
// ┃  (Last Move)    ┃ (Now Turn)  ┃  (Timer/Progress Bar)   │ (Play Time)                           ┃
// ┠─────────────────╂─────────────╊━━━━━━━━━━━━━━━━━━━━━━━━━━┷━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
// ┃  (Last Move     ┃ (Turn       ┃ (System Log Area)                                                ┃
// ┃   Display)      ┃  Display)   ┃                                                                  ┃
// ┗━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

// 박스 먼저 그리기 (박스를 그리고 세부 조정하는 방향으로)
void ingame_draw_box(const int start_x, const int start_y,
                     const int width, const int height)
{
    // Bold 속성으로 테두리 그리기
    attron(A_BOLD);

    // Top border
    mvaddch(start_y, start_x, ACS_ULCORNER);
    for (int i = 1; i < width - 1; i++)
        addch(ACS_HLINE);
    addch(ACS_URCORNER);

    // Side borders
    for (int j = 1; j < height - 1; j++)
    {
        mvaddch(start_y + j, start_x, ACS_VLINE);
        mvaddch(start_y + j, start_x + width - 1, ACS_VLINE);
    }

    // Bottom border
    mvaddch(start_y + height - 1, start_x, ACS_LLCORNER);
    for (int i = 1; i < width - 1; i++)
        addch(ACS_HLINE);
    addch(ACS_LRCORNER);

    // Bold 속성 해제
    attroff(A_BOLD);
}

void ingame_border_draw(void)
{
    // Bold 속성으로 테두리 그리기
    attron(A_BOLD);

    // ========================================
    // Row 0: 최상단 테두리
    // ========================================
    mvaddstr(0, 0, "┏");
    for (int i = 1; i < 50; i++)
        addstr("━");
    addstr("┳");
    for (int i = 51; i < 61; i++)
        addstr("━");
    addstr("┯");
    for (int i = 62; i < 76; i++)
        addstr("━");
    addstr("┯");
    for (int i = 77; i < 88; i++)
        addstr("━");
    addstr("┯");
    for (int i = 89; i < 99; i++)
        addstr("━");
    addstr("┓");

    // ========================================
    // Row 1: Info 영역 상단 (플레이어 이름, Viewers, PING, PORT)
    // ========================================
    mvaddstr(1, 0, "┃");
    mvaddstr(1, 50, "┃");
    mvaddstr(1, 99, "┃");

    // ========================================
    // Row 2: Board/Chat 구분선
    // ========================================
    mvaddstr(2, 0, "┃");
    mvaddstr(2, 50, "┣");
    for (int i = 51; i < 61; i++)
        addstr("━");
    addstr("┷");
    for (int i = 62; i < 76; i++)
        addstr("━");
    addstr("┷");
    for (int i = 77; i < 88; i++)
        addstr("━");
    addstr("┷");
    for (int i = 89; i < 99; i++)
        addstr("━");
    addstr("┫");

    // ========================================
    // Row 3-22: Board 영역 왼쪽 테두리 + Chat 영역 오른쪽 테두리
    // ========================================
    for (int row = 3; row <= 22; row++)
    {
        mvaddstr(row, 0, "┃");
        mvaddstr(row, 50, "┃");
        mvaddstr(row, 99, "┃");
    }

    // ========================================
    // Row 23: Chat Input 구분선
    // ========================================
    mvaddstr(23, 0, "┃");
    mvaddstr(23, 50, "┣");
    for (int i = 51; i < 91; i++)
        addstr("━");
    addstr("┯");
    for (int i = 92; i < 99; i++)
        addstr("━");
    addstr("┫");

    // ========================================
    // Row 24: Chat Input 영역
    // ========================================
    mvaddstr(24, 0, "┃");
    mvaddstr(24, 50, "┃");
    mvaddstr(24, 91, "│");
    mvaddstr(24, 99, "┃");

    // ========================================
    // Row 25: Bottom 영역 상단 구분선
    // ========================================
    mvaddstr(25, 0, "┣");
    for (int i = 1; i < 18; i++)
        addstr("━");
    addstr("┳");
    for (int i = 19; i < 32; i++)
        addstr("━");
    addstr("┳");
    for (int i = 33; i < 50; i++)
        addstr("━");
    addstr("┻");
    for (int i = 51; i < 81; i++)
        addstr("━");
    addstr("┯");
    for (int i = 82; i < 99; i++)
        addstr("━");
    addstr("┫");

    // ========================================
    // Row 26: Bottom 영역 (Last Move, Now Turn, Timer, Play Time)
    // ========================================
    mvaddstr(26, 0, "┃");
    mvaddstr(26, 18, "┃");
    mvaddstr(26, 32, "┃");
    mvaddstr(26, 81, "│");
    mvaddstr(26, 99, "┃");

    // ========================================
    // Row 27: Bottom 영역 중간 구분선
    // ========================================
    mvaddstr(27, 0, "┠");
    for (int i = 1; i < 18; i++)
        addstr("─");
    addstr("╂");
    for (int i = 19; i < 32; i++)
        addstr("─");
    addstr("╊");
    for (int i = 33; i < 81; i++)
        addstr("━");
    addstr("┷");
    for (int i = 82; i < 99; i++)
        addstr("━");
    addstr("┫");

    // ========================================
    // Row 28: Log 영역 (Last Move Display, Turn Display, System Log)
    // ========================================
    mvaddstr(28, 0, "┃");
    mvaddstr(28, 18, "┃");
    mvaddstr(28, 32, "┃");
    mvaddstr(28, 99, "┃");

    // ========================================
    // Row 29: 최하단 테두리
    // ========================================
    mvaddstr(29, 0, "┗");
    for (int i = 1; i < 18; i++)
        addstr("━");
    addstr("┻");
    for (int i = 19; i < 32; i++)
        addstr("━");
    addstr("┻");
    for (int i = 33; i < 99; i++)
        addstr("━");
    addstr("┛");

    attroff(A_BOLD);

    refresh();
}

// Board 영역 테두리만 재그리기
void ingame_border_redraw_board_area(void)
{
    attron(A_BOLD);

    // 왼쪽 세로 테두리
    for (int row = 0; row <= 25; row++)
    {
        mvaddstr(row, 0, "┃");
    }

    // Row 0 상단
    mvaddstr(0, 0, "┏");
    for (int i = 1; i < 50; i++)
        addstr("━");
    addstr("┳");

    // Row 25 하단 구분선
    mvaddstr(25, 0, "┣");
    for (int i = 1; i < 18; i++)
        addstr("━");
    addstr("┳");
    for (int i = 19; i < 32; i++)
        addstr("━");
    addstr("┳");
    for (int i = 33; i < 50; i++)
        addstr("━");
    addstr("┻");

    attroff(A_BOLD);
    refresh();
}

// Info 영역 테두리만 재그리기
void ingame_border_redraw_info_area(void)
{
    attron(A_BOLD);

    // Row 0 상단
    mvaddstr(0, 50, "┳");
    for (int i = 51; i < 61; i++)
        addstr("━");
    addstr("┯");
    for (int i = 62; i < 76; i++)
        addstr("━");
    addstr("┯");
    for (int i = 77; i < 88; i++)
        addstr("━");
    addstr("┯");
    for (int i = 89; i < 99; i++)
        addstr("━");
    addstr("┓");

    // Row 1
    mvaddstr(1, 50, "┃");
    mvaddstr(1, 99, "┃");

    // Row 2 구분선
    mvaddstr(2, 50, "┣");
    for (int i = 51; i < 61; i++)
        addstr("━");
    addstr("┷");
    for (int i = 62; i < 76; i++)
        addstr("━");
    addstr("┷");
    for (int i = 77; i < 88; i++)
        addstr("━");
    addstr("┷");
    for (int i = 89; i < 99; i++)
        addstr("━");
    addstr("┫");

    attroff(A_BOLD);
    refresh();
}

// Chat 영역 테두리만 재그리기
void ingame_border_redraw_chat_area(void)
{
    attron(A_BOLD);

    // Chat 영역 세로 테두리 (Row 3-22)
    for (int row = 3; row <= 22; row++)
    {
        mvaddstr(row, 50, "┃");
        mvaddstr(row, 99, "┃");
    }

    // Chat Input 구분선 (Row 23)
    mvaddstr(23, 50, "┣");
    for (int i = 51; i < 91; i++)
        addstr("━");
    addstr("┯");
    for (int i = 92; i < 99; i++)
        addstr("━");
    addstr("┫");

    // Chat Input 영역 (Row 24)
    mvaddstr(24, 50, "┃");
    mvaddstr(24, 91, "│");
    mvaddstr(24, 99, "┃");

    attroff(A_BOLD);
    refresh();
}

// Bottom 영역 테두리만 재그리기
void ingame_border_redraw_bottom_area(void)
{
    attron(A_BOLD);

    // Row 25 상단 구분선
    mvaddstr(25, 0, "┣");
    for (int i = 1; i < 18; i++)
        addstr("━");
    addstr("┳");
    for (int i = 19; i < 32; i++)
        addstr("━");
    addstr("┳");
    for (int i = 33; i < 50; i++)
        addstr("━");
    addstr("┻");
    for (int i = 51; i < 81; i++)
        addstr("━");
    addstr("┯");
    for (int i = 82; i < 99; i++)
        addstr("━");
    addstr("┫");

    // Row 26
    mvaddstr(26, 0, "┃");
    mvaddstr(26, 18, "┃");
    mvaddstr(26, 32, "┃");
    mvaddstr(26, 81, "│");
    mvaddstr(26, 99, "┃");

    // Row 27 중간 구분선
    mvaddstr(27, 0, "┠");
    for (int i = 1; i < 18; i++)
        addstr("─");
    addstr("╂");
    for (int i = 19; i < 32; i++)
        addstr("─");
    addstr("╊");
    for (int i = 33; i < 81; i++)
        addstr("━");
    addstr("┷");
    for (int i = 82; i < 99; i++)
        addstr("━");
    addstr("┫");

    // Row 28
    mvaddstr(28, 0, "┃");
    mvaddstr(28, 18, "┃");
    mvaddstr(28, 32, "┃");
    mvaddstr(28, 99, "┃");

    // Row 29 최하단 테두리
    mvaddstr(29, 0, "┗");
    for (int i = 1; i < 18; i++)
        addstr("━");
    addstr("┻");
    for (int i = 19; i < 32; i++)
        addstr("━");
    addstr("┻");
    for (int i = 33; i < 99; i++)
        addstr("━");
    addstr("┛");

    attroff(A_BOLD);
    refresh();
}

// 전체 Border 재그리기
void ingame_border_redraw_all(void)
{
    ingame_border_draw();
}
