// System / External Libraries
#include <stdio.h>
#include <ncurses.h>

// Custom Code / Headers
#include "application/app_config.h" // Application Layer
// Domain Layer
// Infrastructure Layer
#include "presentation/check_screen_size.h" // Presentation Layer
#include "presentation/cli/argument_parser.h"

int main(int argc, char *argv[])
{
    // 명령줄 인수 파싱
    AppConfig cfg;
    parse_arguments(argc, argv, &cfg);

    // ncurses 초기화
    initscr();

    // 120 * 30 이상일 때까지 대기
    if (!cfg.force_small_shell)
    {
        check_screen_size(120, 30);
    }
    clear();

    // 인트로 표시

    // ncurses 종료
    endwin();

    return 0;
}
