// presentation/check_screen_size.c
/*
터미널의 사이즈를 감지하고 특정 사이즈 이상일 때 코드를 종료합니다.
이는 프로그램이 적절한 화면 크기에서 실행되도록 보장하기 위함입니다.

특정 크기가 될 때까지 코드에서 대기합니다.
*/

// presentation/check_screen_size.c

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

void check_screen_size(int min_width, int min_height)
{
    int height, width;
    int frame = 0;
    char spinner[] = {'|', '/', '-', '\\'};

    nodelay(stdscr, TRUE); // getch()를 논블로킹으로
    curs_set(0);           // 커서 숨기기

    while (1)
    {
        getmaxyx(stdscr, height, width);

        if (width >= min_width && height >= min_height)
        {
            clear();
            nodelay(stdscr, FALSE);
            curs_set(1);
            return;
        }

        clear();
        mvprintw(0, 0, "Terminal size is too small!");
        mvprintw(1, 0, "Please resize the terminal...");
        mvprintw(3, 0, "Minimum size: %dx%d", min_width, min_height);
        mvprintw(4, 0, "Current size: %dx%d", width, height);

        // Loading animation
        mvprintw(6, 0, "Waiting for proper terminal size... %c",
                 spinner[frame % 4]);

        refresh();

        frame++;
        napms(120); // 120ms 딜레이로 부드러운 애니메이션
    }
}
