#include <stdio.h>
#include <unistd.h>
#include <locale.h>
#include <ncurses.h>
#include "../ui/ingame_border.h"
#include "../ui/theme.h"

// 테스트: ingame_border.c 모듈
// 인게임 레이아웃 테두리가 올바르게 그려지는지 확인

int main(void)
{
    // UTF-8 로케일 설정 (필수!)
    setlocale(LC_ALL, "");

    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    // 터미널 크기 확인
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    if (max_x < 100 || max_y < 30)
    {
        endwin();
        fprintf(stderr, "Terminal too small! Need at least 100x30, got %dx%d\n", max_x, max_y);
        return 1;
    }

    // 색상 초기화
    if (has_colors())
    {
        start_color();
        theme_init(THEME_WHITE);
    }

    // 테스트 1: 전체 Border 그리기
    clear();
    mvprintw(0, 0, "Test 1: ingame_border_draw() - Press any key to continue...");
    refresh();
    getch();

    clear();
    ingame_border_draw();
    mvprintw(max_y - 1, 0, "ingame_border_draw() complete. Press any key...");
    refresh();
    getch();

    // 테스트 2: 전체 Border 재그리기
    clear();
    mvprintw(0, 0, "Test 2: ingame_border_redraw_all() - Press any key to continue...");
    refresh();
    getch();

    clear();
    ingame_border_redraw_all();
    mvprintw(max_y - 1, 0, "ingame_border_redraw_all() complete. Press any key...");
    refresh();
    getch();

    // 테스트 3: 각 영역 재그리기
    clear();
    mvprintw(0, 0, "Test 3: Area-specific redraw functions - Press any key to continue...");
    refresh();
    getch();

    clear();
    ingame_border_draw();
    mvprintw(max_y - 1, 0, "Base layout drawn. Press key to test board area redraw...");
    refresh();
    getch();

    ingame_border_redraw_board_area();
    mvprintw(max_y - 1, 0, "Board area redrawn. Press key to test info area...");
    refresh();
    getch();

    ingame_border_redraw_info_area();
    mvprintw(max_y - 1, 0, "Info area redrawn. Press key to test chat area...");
    refresh();
    getch();

    ingame_border_redraw_chat_area();
    mvprintw(max_y - 1, 0, "Chat area redrawn. Press any key to exit...");
    refresh();
    getch();

    // 종료
    endwin();
    printf("ingame_border test completed successfully!\n");

    return 0;
}
