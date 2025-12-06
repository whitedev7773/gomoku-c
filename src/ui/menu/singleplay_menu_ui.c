#include "singleplay_menu_ui.h"
#include "../core/input_handler.h"
#include "../core/theme.h"
#include "../core/ui_manager.h"
#include <string.h>
#include <locale.h>

// ==========================================
// 난이도 선택 UI
// ==========================================

void difficulty_select_ui_init(DifficultySelectUI *ui)
{
    if (!ui)
        return;

    ui->selected = 0; // Easy가 기본값
    ui->option_count = 2;
}

void difficulty_select_ui_render(WINDOW *win, const DifficultySelectUI *ui)
{
    if (!win || !ui)
        return;

    wclear(win);

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    // 외곽 박스
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    box(win, 0, 0);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 타이틀
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(win, 2, (max_x - 24) / 2, "╔═════════════════════╗");
    mvwprintw(win, 3, (max_x - 24) / 2, "║  SELECT DIFFICULTY  ║");
    mvwprintw(win, 4, (max_x - 24) / 2, "╚═════════════════════╝");
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 옵션 박스
    int box_y = 8;
    int box_w = 60;
    int box_h = 10;
    int box_x = (max_x - box_w) / 2;

    // 옵션 박스 테두리
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwaddch(win, box_y, box_x, ACS_ULCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y, box_x + box_w - 1, ACS_URCORNER);

    for (int j = 1; j < box_h - 1; j++)
    {
        mvwaddch(win, box_y + j, box_x, ACS_VLINE);
        mvwaddch(win, box_y + j, box_x + box_w - 1, ACS_VLINE);
    }

    mvwaddch(win, box_y + box_h - 1, box_x, ACS_LLCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y + box_h - 1, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y + box_h - 1, box_x + box_w - 1, ACS_LRCORNER);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 옵션들 (가로 정렬)
    int option_y = box_y + 4;
    int option_width = 20;
    int gap = 8;
    int total_width = option_width * 2 + gap;
    int start_x = box_x + (box_w - total_width) / 2;

    // Easy 옵션
    int easy_x = start_x;
    if (ui->selected == 0)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_REVERSE);
        mvwprintw(win, option_y - 1, easy_x, "╔══════════════════╗");
        mvwprintw(win, option_y + 0, easy_x, "║     ▶ EASY ◀     ║");
        mvwprintw(win, option_y + 1, easy_x, "╚══════════════════╝");
        wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_REVERSE);
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, option_y - 1, easy_x, "┌──────────────────┐");
        mvwprintw(win, option_y + 0, easy_x, "│       EASY       │");
        mvwprintw(win, option_y + 1, easy_x, "└──────────────────┘");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
    }

    // Hard 옵션
    int hard_x = start_x + option_width + gap;
    if (ui->selected == 1)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_REVERSE);
        mvwprintw(win, option_y - 1, hard_x, "╔══════════════════╗");
        mvwprintw(win, option_y + 0, hard_x, "║     ▶ HARD ◀     ║");
        mvwprintw(win, option_y + 1, hard_x, "╚══════════════════╝");
        wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_REVERSE);
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, option_y - 1, hard_x, "┌──────────────────┐");
        mvwprintw(win, option_y + 0, hard_x, "│       HARD       │");
        mvwprintw(win, option_y + 1, hard_x, "└──────────────────┘");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
    }

    // 설명 텍스트
    wattron(win, COLOR_PAIR(COLOR_PAIR_INFO));
    if (ui->selected == 0)
    {
        mvwprintw(win, box_y + box_h + 1, (max_x - 44) / 2, "  Basic AI with heuristic-based evaluation  ");
    }
    else
    {
        mvwprintw(win, box_y + box_h + 1, (max_x - 44) / 2, "Advanced AI with Minimax + Alpha-Beta Pruning");
    }
    wattroff(win, COLOR_PAIR(COLOR_PAIR_INFO));

    // 안내 메시지
    wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
    mvwprintw(win, max_y - 3, (max_x - 40) / 2, "←→ / ↑↓: Select    ↵: Confirm    Q: Back");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));

    wrefresh(win);
}

void difficulty_select_ui_move(DifficultySelectUI *ui, int direction)
{
    if (!ui)
        return;

    ui->selected += direction;

    // 순환
    if (ui->selected < 0)
        ui->selected = ui->option_count - 1;
    else if (ui->selected >= ui->option_count)
        ui->selected = 0;
}

AIDifficulty difficulty_select_ui_get_selected(const DifficultySelectUI *ui)
{
    if (!ui)
        return AI_EASY;
    return (ui->selected == 0) ? AI_EASY : AI_HARD;
}

// ==========================================
// 규칙 선택 UI
// ==========================================

void rule_select_ui_init(RuleSelectUI *ui)
{
    if (!ui)
        return;

    ui->selected = 1; // Renju가 기본값
    ui->option_count = 2;
}

void rule_select_ui_render(WINDOW *win, const RuleSelectUI *ui)
{
    if (!win || !ui)
        return;

    wclear(win);

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    // 외곽 박스
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    box(win, 0, 0);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 타이틀
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(win, 2, (max_x - 20) / 2, "╔═════════════════╗");
    mvwprintw(win, 3, (max_x - 20) / 2, "║   SELECT RULE   ║");
    mvwprintw(win, 4, (max_x - 20) / 2, "╚═════════════════╝");
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 옵션 박스
    int box_y = 8;
    int box_w = 60;
    int box_h = 10;
    int box_x = (max_x - box_w) / 2;

    // 옵션 박스 테두리
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwaddch(win, box_y, box_x, ACS_ULCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y, box_x + box_w - 1, ACS_URCORNER);

    for (int j = 1; j < box_h - 1; j++)
    {
        mvwaddch(win, box_y + j, box_x, ACS_VLINE);
        mvwaddch(win, box_y + j, box_x + box_w - 1, ACS_VLINE);
    }

    mvwaddch(win, box_y + box_h - 1, box_x, ACS_LLCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y + box_h - 1, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y + box_h - 1, box_x + box_w - 1, ACS_LRCORNER);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 옵션들 (가로 정렬)
    int option_y = box_y + 4;
    int option_width = 20;
    int gap = 8;
    int total_width = option_width * 2 + gap;
    int start_x = box_x + (box_w - total_width) / 2;

    // Standard 옵션
    int std_x = start_x;
    if (ui->selected == 0)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_REVERSE);
        mvwprintw(win, option_y - 1, std_x, "╔══════════════════╗");
        mvwprintw(win, option_y + 0, std_x, "║   ▶ STANDARD ◀   ║");
        mvwprintw(win, option_y + 1, std_x, "╚══════════════════╝");
        wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_REVERSE);
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, option_y - 1, std_x, "┌──────────────────┐");
        mvwprintw(win, option_y + 0, std_x, "│     STANDARD     │");
        mvwprintw(win, option_y + 1, std_x, "└──────────────────┘");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
    }

    // Renju 옵션
    int renju_x = start_x + option_width + gap;
    if (ui->selected == 1)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_REVERSE);
        mvwprintw(win, option_y - 1, renju_x, "╔══════════════════╗");
        mvwprintw(win, option_y + 0, renju_x, "║    ▶ RENJU ◀     ║");
        mvwprintw(win, option_y + 1, renju_x, "╚══════════════════╝");
        wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_REVERSE);
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, option_y - 1, renju_x, "┌──────────────────┐");
        mvwprintw(win, option_y + 0, renju_x, "│      RENJU       │");
        mvwprintw(win, option_y + 1, renju_x, "└──────────────────┘");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
    }

    // 설명 텍스트
    wattron(win, COLOR_PAIR(COLOR_PAIR_INFO));
    if (ui->selected == 0)
    {
        mvwprintw(win, box_y + box_h + 1, (max_x - 42) / 2, "  No forbidden moves - Classic Gomoku rules ");
    }
    else
    {
        mvwprintw(win, box_y + box_h + 1, (max_x - 48) / 2, "Forbidden moves for BLACK (3-3, 4-4, Overline)");
    }
    wattroff(win, COLOR_PAIR(COLOR_PAIR_INFO));

    // 안내 메시지
    wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
    mvwprintw(win, max_y - 3, (max_x - 40) / 2, "←→ / ↑↓: Select    ↵: Confirm    Q: Back");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));

    wrefresh(win);
}

void rule_select_ui_move(RuleSelectUI *ui, int direction)
{
    if (!ui)
        return;

    ui->selected += direction;

    // 순환
    if (ui->selected < 0)
        ui->selected = ui->option_count - 1;
    else if (ui->selected >= ui->option_count)
        ui->selected = 0;
}

GameRule rule_select_ui_get_selected(const RuleSelectUI *ui)
{
    if (!ui)
        return RULE_RENJU;
    return (ui->selected == 0) ? RULE_STANDARD : RULE_RENJU;
}

// ==========================================
// 통합 선택 화면
// ==========================================

int singleplay_select_difficulty(void)
{
    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    if (has_colors())
    {
        start_color();
        ThemeType saved_theme = theme_load_from_config();
        theme_init(saved_theme);
    }

    wresize(stdscr, UI_MIN_HEIGHT, UI_MIN_WIDTH);
    WINDOW *select_win = newwin(UI_MIN_HEIGHT, UI_MIN_WIDTH, 0, 0);
    keypad(select_win, TRUE);

    InputHandler input_handler;
    input_handler_init(&input_handler);

    DifficultySelectUI ui;
    difficulty_select_ui_init(&ui);

    bool running = true;
    int result = -1;

    while (running)
    {
        difficulty_select_ui_render(select_win, &ui);

        InputEvent event = input_handler_get_event(&input_handler, select_win);

        switch (event.action)
        {
        case INPUT_MOVE_UP:
        case INPUT_MOVE_LEFT:
            difficulty_select_ui_move(&ui, -1);
            break;

        case INPUT_MOVE_DOWN:
        case INPUT_MOVE_RIGHT:
            difficulty_select_ui_move(&ui, 1);
            break;

        case INPUT_PLACE_STONE:
            result = ui.selected;
            running = false;
            break;

        case INPUT_QUIT:
            result = -1;
            running = false;
            break;

        default:
            break;
        }
    }

    input_handler_cleanup(&input_handler);
    delwin(select_win);
    endwin();

    return result;
}

int singleplay_select_rule(void)
{
    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    if (has_colors())
    {
        start_color();
        ThemeType saved_theme = theme_load_from_config();
        theme_init(saved_theme);
    }

    wresize(stdscr, UI_MIN_HEIGHT, UI_MIN_WIDTH);
    WINDOW *select_win = newwin(UI_MIN_HEIGHT, UI_MIN_WIDTH, 0, 0);
    keypad(select_win, TRUE);

    InputHandler input_handler;
    input_handler_init(&input_handler);

    RuleSelectUI ui;
    rule_select_ui_init(&ui);

    bool running = true;
    int result = -1;

    while (running)
    {
        rule_select_ui_render(select_win, &ui);

        InputEvent event = input_handler_get_event(&input_handler, select_win);

        switch (event.action)
        {
        case INPUT_MOVE_UP:
        case INPUT_MOVE_LEFT:
            rule_select_ui_move(&ui, -1);
            break;

        case INPUT_MOVE_DOWN:
        case INPUT_MOVE_RIGHT:
            rule_select_ui_move(&ui, 1);
            break;

        case INPUT_PLACE_STONE:
            result = ui.selected;
            running = false;
            break;

        case INPUT_QUIT:
            result = -1;
            running = false;
            break;

        default:
            break;
        }
    }

    input_handler_cleanup(&input_handler);
    delwin(select_win);
    endwin();

    return result;
}

int singleplay_run_settings(AIDifficulty *difficulty, GameRule *rule)
{
    if (!difficulty || !rule)
        return -1;

    // 1. 난이도 선택
    int diff_result = singleplay_select_difficulty();
    if (diff_result < 0)
        return -1;

    *difficulty = (diff_result == 0) ? AI_EASY : AI_HARD;

    // 2. 규칙 선택
    int rule_result = singleplay_select_rule();
    if (rule_result < 0)
        return -1;

    *rule = (rule_result == 0) ? RULE_STANDARD : RULE_RENJU;

    return 0;
}
