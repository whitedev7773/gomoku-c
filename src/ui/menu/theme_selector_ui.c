#include "theme_selector_ui.h"
#include "../core/input_handler.h"
#include "../core/ui_manager.h"
#include <string.h>

void theme_selector_init(ThemeSelectorUI *selector)
{
    if (!selector)
        return;

    selector->selected_theme = theme_get_current();
    selector->theme_count = theme_get_count();
}

void theme_selector_render(WINDOW *win, const ThemeSelectorUI *selector)
{
    if (!win || !selector)
        return;

    wclear(win);

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    // 전체 윈도우 Border
    int box_w = UI_MIN_WIDTH;
    int box_h = UI_MIN_HEIGHT;
    int start_y = 0;
    int start_x = 0;

    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    // Top border
    mvwaddch(win, start_y, start_x, ACS_ULCORNER);
    for (int i = 1; i < box_w - 1; ++i)
        mvwaddch(win, start_y, start_x + i, ACS_HLINE);
    mvwaddch(win, start_y, start_x + box_w - 1, ACS_URCORNER);

    // Side borders
    for (int j = 1; j < box_h - 1; ++j)
    {
        mvwaddch(win, start_y + j, start_x, ACS_VLINE);
        mvwaddch(win, start_y + j, start_x + box_w - 1, ACS_VLINE);
    }

    // Bottom border
    mvwaddch(win, start_y + box_h - 1, start_x, ACS_LLCORNER);
    for (int i = 1; i < box_w - 1; ++i)
        mvwaddch(win, start_y + box_h - 1, start_x + i, ACS_HLINE);
    mvwaddch(win, start_y + box_h - 1, start_x + box_w - 1, ACS_LRCORNER);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 타이틀
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(win, 2, (max_x - 20) / 2, "=== THEME SELECTOR ===");
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 테마 목록 박스
    int list_box_y = 5;
    int list_box_x = 25;
    int list_box_w = 50;
    int list_box_h = 12;

    // 박스 그리기 (주색상 적용)
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwaddch(win, list_box_y, list_box_x, ACS_ULCORNER);
    for (int i = 1; i < list_box_w - 1; i++)
        mvwaddch(win, list_box_y, list_box_x + i, ACS_HLINE);
    mvwaddch(win, list_box_y, list_box_x + list_box_w - 1, ACS_URCORNER);

    for (int j = 1; j < list_box_h - 1; j++)
    {
        mvwaddch(win, list_box_y + j, list_box_x, ACS_VLINE);
        mvwaddch(win, list_box_y + j, list_box_x + list_box_w - 1, ACS_VLINE);
    }

    mvwaddch(win, list_box_y + list_box_h - 1, list_box_x, ACS_LLCORNER);
    for (int i = 1; i < list_box_w - 1; i++)
        mvwaddch(win, list_box_y + list_box_h - 1, list_box_x + i, ACS_HLINE);
    mvwaddch(win, list_box_y + list_box_h - 1, list_box_x + list_box_w - 1, ACS_LRCORNER);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 테마 목록 표시
    int list_y = list_box_y + 2;
    int list_x = list_box_x + 5;

    for (int i = 0; i < selector->theme_count; i++)
    {
        wmove(win, list_y + i, list_x);

        if (i == selector->selected_theme)
        {
            wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_REVERSE);
            wprintw(win, " ▶  %-30s  ", theme_get_name(i));
            wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_REVERSE);
        }
        else
        {
            wprintw(win, "    %-30s  ", theme_get_name(i));
        }
    }

    // 테마 미리보기
    int preview_y = list_box_y + list_box_h + 2;
    int preview_x = list_box_x;

    wattron(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(win, preview_y, preview_x, "Preview:");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));

    wattron(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(win, preview_y + 1, preview_x + 2, "DEFAULT ");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));

    wattron(win, COLOR_PAIR(COLOR_PAIR_BLACK_STONE));
    wprintw(win, "BLACK ");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_BLACK_STONE));

    wattron(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE));
    wprintw(win, "WHITE ");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE));

    wattron(win, COLOR_PAIR(COLOR_PAIR_SYSTEM));
    wprintw(win, "SYSTEM ");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_SYSTEM));

    wattron(win, COLOR_PAIR(COLOR_PAIR_OPPONENT));
    wprintw(win, "OPPONENT ");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_OPPONENT));

    wattron(win, COLOR_PAIR(COLOR_PAIR_INFO));
    wprintw(win, "INFO");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_INFO));

    // 안내 메시지
    wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
    mvwprintw(win, max_y - 3, (max_x - 38) / 2, "↑↓: Select Theme    ↵: Apply    Q: Back");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));

    wrefresh(win);
}

void theme_selector_move(ThemeSelectorUI *selector, int direction)
{
    if (!selector)
        return;

    selector->selected_theme += direction;

    if (selector->selected_theme < 0)
    {
        selector->selected_theme = selector->theme_count - 1;
    }
    else if (selector->selected_theme >= selector->theme_count)
    {
        selector->selected_theme = 0;
    }

    // 선택한 테마 미리보기 적용
    theme_set(selector->selected_theme);
}

ThemeType theme_selector_get_selected(const ThemeSelectorUI *selector)
{
    if (!selector)
        return THEME_WHITE;
    return selector->selected_theme;
}

ThemeType theme_selector_run(void)
{
    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    if (has_colors())
    {
        start_color();
    }

    // 현재 테마로 초기화
    ThemeType current_theme = theme_get_current();
    if (!g_theme_manager.initialized)
    {
        theme_init(current_theme);
    }

    // UI_MIN_WIDTH x UI_MIN_HEIGHT 윈도우 생성
    WINDOW *selector_win = newwin(UI_MIN_HEIGHT, UI_MIN_WIDTH, 0, 0);
    keypad(selector_win, TRUE);

    // 게임패드 입력 핸들러 초기화
    InputHandler input_handler;
    input_handler_init(&input_handler);

    ThemeSelectorUI selector;
    theme_selector_init(&selector);

    bool running = true;
    ThemeType selected_theme = current_theme;

    // 초기 렌더링 한 번만 수행
    theme_selector_render(selector_win, &selector);

    while (running)
    {
        // 블로킹으로 키 입력 대기 (키가 눌릴 때만 처리 및 재렌더링)
        InputEvent event = input_get_event(selector_win);

        if (event.action == INPUT_NONE)
        {
            // 입력이 없는 경우는 거의 없음(블로킹 호출이므로), 루프 계속
            continue;
        }

        switch (event.action)
        {
        case INPUT_MOVE_UP:
            theme_selector_move(&selector, -1);
            // 선택 변경이 있으면 화면 갱신
            theme_selector_render(selector_win, &selector);
            break;

        case INPUT_MOVE_DOWN:
            theme_selector_move(&selector, 1);
            theme_selector_render(selector_win, &selector);
            break;

        case INPUT_PLACE_STONE:
            // 테마 적용
            selected_theme = theme_selector_get_selected(&selector);
            theme_set(selected_theme);

            // 설정 파일에 저장
            theme_save_to_config();

            running = false;
            break;

        case INPUT_QUIT:
            // 이전 테마로 복원
            theme_set(current_theme);
            selected_theme = current_theme;
            running = false;
            break;

        default:
            break;
        }
    }

    input_handler_cleanup(&input_handler);
    delwin(selector_win);
    endwin();

    return selected_theme;
}
