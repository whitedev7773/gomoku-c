#include "game_info_ui.h"
#include "board_ui.h"
#include <string.h>

void game_info_ui_init(GameInfoUI *ui) {
    if (!ui) return;
    ui->game_start_time = time(NULL);
    ui->prev_last_row = -1;
    ui->prev_last_col = -1;
    ui->prev_current_player = EMPTY;
    ui->prev_elapsed_seconds = -1;
}

int game_info_ui_get_elapsed_seconds(const GameInfoUI *ui) {
    if (!ui) return 0;
    time_t now = time(NULL);
    return (int)difftime(now, ui->game_start_time);
}

void game_info_ui_draw_last_move(WINDOW *win, const Board *board) {
    if (!win || !board) return;

    Position last_move = board_get_last_move(board);

    mvwprintw(win, 0, 4, "LAST MOVE");

    if (last_move.row == -1 || last_move.col == -1) {
        mvwprintw(win, 1, 2, "  ----  ----");
        return;
    }

    Stone last_stone = board_get_stone(board, last_move.row, last_move.col);

    // Draw stone ASCII art
    int x = 2;
    if (last_stone == BLACK) {
        mvwprintw(win, 1, x, "  ┏━━┓ ┏━━┓┏━━┓");
        mvwprintw(win, 2, x, "  ┣━━┫ ┃  ┃┃  ┃");
        mvwprintw(win, 3, x, "  ╹  ╹ ┗━━┛┗━━┛");
    } else {
        mvwprintw(win, 1, x, "  ┏━━┓  ━┓");
        mvwprintw(win, 2, x, "  ┣━━┛   ┃");
        mvwprintw(win, 3, x, "  ╹    ━━┻━");
    }
}

void game_info_ui_draw_current_turn(WINDOW *win, Stone current_player) {
    if (!win) return;

    mvwprintw(win, 0, 22, "NOW  TURN");

    int x = 21;
    if (current_player == BLACK) {
        mvwprintw(win, 1, x, "  ┏━━┓ ┏━━┓┏━━┓");
        mvwprintw(win, 2, x, "  ┣━━┫ ┃  ┃┃  ┃");
        mvwprintw(win, 3, x, "  ╹  ╹ ┗━━┛┗━━┛");
    } else {
        mvwprintw(win, 1, x, "  ┏━━┓  ━┓");
        mvwprintw(win, 2, x, "  ┣━━┛   ┃");
        mvwprintw(win, 3, x, "  ╹    ━━┻━");
    }
}

void game_info_ui_draw_timer(WINDOW *win, int remaining_seconds) {
    if (!win) return;

    int filled = (remaining_seconds * 20) / 20;
    if (filled > 20) filled = 20;

    mvwprintw(win, 0, 38, "  TIME: %2ds  ", remaining_seconds);

    wmove(win, 0, 52);
    for (int i = 0; i < filled; i++) {
        wprintw(win, "█");
    }
    for (int i = filled; i < 20; i++) {
        wprintw(win, "░");
    }
}

void game_info_ui_draw_play_time(WINDOW *win, const GameInfoUI *ui) {
    if (!win || !ui) return;

    int elapsed = game_info_ui_get_elapsed_seconds(ui);
    int minutes = elapsed / 60;
    int seconds = elapsed % 60;

    mvwprintw(win, 0, 63, " PLAY TIME:  %02d:%02d ", minutes, seconds);
}

void game_info_ui_draw_buttons(WINDOW *win) {
    if (!win) return;

    mvwprintw(win, 1, 76, "            ");
    mvwprintw(win, 2, 76, "  REQ UNDO  ");
    mvwprintw(win, 3, 76, "            ");

    mvwprintw(win, 1, 88, "            ");
    mvwprintw(win, 2, 88, "   RESIGN   ");
    mvwprintw(win, 3, 88, "            ");
}

// 초기 렌더링 (테두리만 - 한 번만 호출)
void game_info_ui_init_bottom(WINDOW *win) {
    if (!win) return;

    wclear(win);

    // Draw fixed size border
    wattron(win, A_BOLD);
    mvwaddch(win, 0, 0, ACS_ULCORNER);
    for (int i = 1; i < 99; i++) {
        mvwaddch(win, 0, i, ACS_HLINE);
    }
    mvwaddch(win, 0, 99, ACS_URCORNER);

    for (int j = 1; j < 3; j++) {
        mvwaddch(win, j, 0, ACS_VLINE);
        mvwaddch(win, j, 99, ACS_VLINE);
    }

    mvwaddch(win, 3, 0, ACS_LLCORNER);
    for (int i = 1; i < 99; i++) {
        mvwaddch(win, 3, i, ACS_HLINE);
    }
    mvwaddch(win, 3, 99, ACS_LRCORNER);
    wattroff(win, A_BOLD);

    wrefresh(win);
}

void game_info_ui_render_bottom(WINDOW *win, const Board *board,
                                 const TurnManager *turn_mgr,
                                 const GameInfoUI *ui) {
    if (!win) return;

    wclear(win);

    // Always render with fixed 100x4 size (BOTTOM_WINDOW_WIDTH x BOTTOM_WINDOW_HEIGHT)
    // Draw fixed size border
    wattron(win, A_BOLD);
    mvwaddch(win, 0, 0, ACS_ULCORNER);
    for (int i = 1; i < 99; i++) {
        mvwaddch(win, 0, i, ACS_HLINE);
    }
    mvwaddch(win, 0, 99, ACS_URCORNER);

    for (int j = 1; j < 3; j++) {
        mvwaddch(win, j, 0, ACS_VLINE);
        mvwaddch(win, j, 99, ACS_VLINE);
    }

    mvwaddch(win, 3, 0, ACS_LLCORNER);
    for (int i = 1; i < 99; i++) {
        mvwaddch(win, 3, i, ACS_HLINE);
    }
    mvwaddch(win, 3, 99, ACS_LRCORNER);
    wattroff(win, A_BOLD);

    if (board && turn_mgr && ui) {
        game_info_ui_draw_last_move(win, board);
        game_info_ui_draw_current_turn(win, turn_manager_get_current_player(turn_mgr));
        game_info_ui_draw_timer(win, turn_manager_get_remaining_time(turn_mgr));
        game_info_ui_draw_play_time(win, ui);
        game_info_ui_draw_buttons(win);
    }

    wrefresh(win);
}

// 최적화된 업데이트 (변경된 부분만)
void game_info_ui_update_bottom(WINDOW *win, const Board *board,
                                 const TurnManager *turn_mgr,
                                 GameInfoUI *ui) {
    if (!win || !board || !turn_mgr || !ui) return;

    bool need_refresh = false;

    // 1. Last move 변경 체크
    Position last_move = board_get_last_move(board);
    if (ui->prev_last_row != last_move.row || ui->prev_last_col != last_move.col) {
        game_info_ui_draw_last_move(win, board);
        ui->prev_last_row = last_move.row;
        ui->prev_last_col = last_move.col;
        need_refresh = true;
    }

    // 2. 현재 턴 변경 체크
    Stone current_player = turn_manager_get_current_player(turn_mgr);
    if (ui->prev_current_player != current_player) {
        game_info_ui_draw_current_turn(win, current_player);
        ui->prev_current_player = current_player;
        need_refresh = true;
    }

    // 3. 타이머는 항상 업데이트 (매 초 변경)
    game_info_ui_draw_timer(win, turn_manager_get_remaining_time(turn_mgr));
    need_refresh = true;

    // 4. 경과 시간 (초 단위로만 체크)
    int elapsed = game_info_ui_get_elapsed_seconds(ui);
    if (ui->prev_elapsed_seconds != elapsed) {
        game_info_ui_draw_play_time(win, ui);
        ui->prev_elapsed_seconds = elapsed;
        need_refresh = true;
    }

    if (need_refresh) {
        wrefresh(win);
    }
}
