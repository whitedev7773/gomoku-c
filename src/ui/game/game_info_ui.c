#include "game_info_ui.h"
#include "board_ui.h"
#include "../../game/core/turn_manager.h"
#include <string.h>
#include <stdio.h>

void game_info_ui_init(GameInfoUI *ui)
{
    if (!ui)
        return;
    ui->game_start_time = time(NULL);
    ui->prev_last_row = -1;
    ui->prev_last_col = -1;
    ui->prev_current_player = EMPTY;
    ui->prev_elapsed_seconds = -1;
    ui->prev_remaining_seconds = -1;
}

int game_info_ui_get_elapsed_seconds(const GameInfoUI *ui)
{
    if (!ui)
        return 0;
    time_t now = time(NULL);
    return (int)difftime(now, ui->game_start_time);
}

void game_info_ui_draw_last_move(WINDOW *win, const Board *board)
{
    // if (!win || !board)
    //     return;

    // Position last_move = board_get_last_move(board);

    // mvwprintw(win, 0, 4, "LAST MOVE");

    // if (last_move.row == -1 || last_move.col == -1)
    // {
    //     mvwprintw(win, 1, 2, "  ----  ----");
    //     return;
    // }

    // Stone last_stone = board_get_stone(board, last_move.row, last_move.col);

    // // Draw stone ASCII art
    // int x = 2;
    // if (last_stone == BLACK)
    // {
    //     mvwprintw(win, 1, x, "  ┏━━┓ ┏━━┓┏━━┓");
    //     mvwprintw(win, 2, x, "  ┣━━┫ ┃  ┃┃  ┃");
    //     mvwprintw(win, 3, x, "  ╹  ╹ ┗━━┛┗━━┛");
    // }
    // else
    // {
    //     mvwprintw(win, 1, x, "  ┏━━┓  ━┓");
    //     mvwprintw(win, 2, x, "  ┣━━┛   ┃");
    //     mvwprintw(win, 3, x, "  ╹    ━━┻━");
    // }
}

void game_info_ui_draw_current_turn(WINDOW *win, Stone current_player)
{
    if (!win)
        return;

    // NOW TURN 영역: bottom_win 내 좌표
    // NOW TURN 박스는 stdscr (18, 24) 시작, bottom_win은 (1, 25) 시작
    // 따라서 bottom_win 내에서 x=17 (18-1), y는 -1 (25-25=0 부터)
    // 제목: row 24 -> bottom_win row -1 (범위 밖, stdscr에 직접 그려야 함)

    // stdscr에 직접 그리기 (bottom_win 범위 밖)
    mvprintw(25, 21, "NOW  TURN");

    int x = 21;
    if (current_player == BLACK)
    {
        mvprintw(27, x, "╻    ╻ ┏╸");
        mvprintw(28, x, "┣━━┓ ┣┫  ");
        mvprintw(29, x, "┗━━┛ ╹ ┗╸");
    }
    else
    {
        mvprintw(27, x, "╻┏┓╻ ━┳━ ");
        mvprintw(28, x, "┃┃┃┃  ┃  ");
        mvprintw(29, x, "┗┛┗┛  ╹  ");
    }
    refresh();
}

void game_info_ui_draw_timer(WINDOW *win, int remaining_seconds)
{
    if (!win)
        return;

    int filled = (remaining_seconds * TURN_TIMEOUT_SECONDS) / TURN_TIMEOUT_SECONDS;
    if (filled > TURN_TIMEOUT_SECONDS)
        filled = TURN_TIMEOUT_SECONDS;

    mvwprintw(win, 0, 35, "TIME: %2ds", remaining_seconds);

    wmove(win, 0, 46);
    for (int i = 0; i < filled; i++)
    {
        wprintw(win, "█");
    }
    for (int i = filled; i < TURN_TIMEOUT_SECONDS; i++)
    {
        wprintw(win, "░");
    }
}

void game_info_ui_draw_play_time(WINDOW *win, const GameInfoUI *ui)
{
    if (!win || !ui)
        return;

    int elapsed = game_info_ui_get_elapsed_seconds(ui);
    int minutes = elapsed / 60;
    int seconds = elapsed % 60;

    mvwprintw(win, 0, 79, " PLAY TIME:  %02d:%02d ", minutes, seconds);
}

void game_info_ui_draw_buttons(WINDOW *win)
{
    if (!win)
        return;

    mvwprintw(win, 1, 76, "            ");
    mvwprintw(win, 2, 76, "  REQ UNDO  ");
    mvwprintw(win, 3, 76, "            ");

    mvwprintw(win, 1, 88, "            ");
    mvwprintw(win, 2, 88, "   RESIGN   ");
    mvwprintw(win, 3, 88, "            ");
}

// 초기 렌더링 (테두리는 ingame_border.c가 담당)
void game_info_ui_init_bottom(WINDOW *win)
{
    if (!win)
        return;

    wclear(win);
    // 테두리는 ingame_border.c에서 stdscr에 그림
    // 여기서는 내용만 그림
    wrefresh(win);
}

// 최적화된 업데이트 (변경된 부분만)
void game_info_ui_update_bottom(WINDOW *win, const Board *board,
                                const TurnManager *turn_mgr,
                                GameInfoUI *ui)
{
    if (!win || !board || !turn_mgr || !ui)
        return;

    bool need_refresh = false;

    // 1. Last move 변경 체크
    Position last_move = board_get_last_move(board);
    if (ui->prev_last_row != last_move.row || ui->prev_last_col != last_move.col)
    {
        game_info_ui_draw_last_move(win, board);
        ui->prev_last_row = last_move.row;
        ui->prev_last_col = last_move.col;
        need_refresh = true;
    }

    // 2. 현재 턴 변경 체크
    Stone current_player = turn_manager_get_current_player(turn_mgr);
    if (ui->prev_current_player != current_player)
    {
        game_info_ui_draw_current_turn(win, current_player);
        ui->prev_current_player = current_player;
        need_refresh = true;
    }

    // 3. 타이머는 초 단위로만 업데이트
    int remaining = turn_manager_get_remaining_time(turn_mgr);
    if (ui->prev_remaining_seconds != remaining)
    {
        game_info_ui_draw_timer(win, remaining);
        ui->prev_remaining_seconds = remaining;
        need_refresh = true;
    }

    // 4. 경과 시간 (초 단위로만 체크)
    int elapsed = game_info_ui_get_elapsed_seconds(ui);
    if (ui->prev_elapsed_seconds != elapsed)
    {
        game_info_ui_draw_play_time(win, ui);
        ui->prev_elapsed_seconds = elapsed;
        need_refresh = true;
    }

    if (need_refresh)
    {
        wrefresh(win);
    }
}

// ============================================
// 선택적 렌더링 함수 (Dirty Flag 기반)
// ============================================

void game_info_ui_selective_render(WINDOW *win, const Board *board,
                                   const TurnManager *turn_mgr,
                                   GameInfoUI *ui,
                                   UIRenderFlags *flags,
                                   bool first_render)
{
    if (!win || !board || !turn_mgr || !ui || !flags)
        return;

    bool need_refresh = false;

    // 첫 렌더링: 버튼 포함 전체 렌더링 (테두리는 ingame_border.c가 담당)
    if (first_render || ui_render_flags_is_set(flags, RENDER_BOTTOM_BORDER))
    {
        wclear(win);

        // 버튼 그리기 (고정)
        game_info_ui_draw_buttons(win);

        // 모든 정보 그리기
        game_info_ui_draw_last_move(win, board);
        game_info_ui_draw_current_turn(win, turn_manager_get_current_player(turn_mgr));
        game_info_ui_draw_timer(win, turn_manager_get_remaining_time(turn_mgr));
        game_info_ui_draw_play_time(win, ui);

        // 상태 갱신
        Position last_move = board_get_last_move(board);
        ui->prev_last_row = last_move.row;
        ui->prev_last_col = last_move.col;
        ui->prev_current_player = turn_manager_get_current_player(turn_mgr);
        ui->prev_remaining_seconds = turn_manager_get_remaining_time(turn_mgr);
        ui->prev_elapsed_seconds = game_info_ui_get_elapsed_seconds(ui);

        ui_render_flags_clear(flags, RENDER_BOTTOM_BORDER);
        need_refresh = true;
    }
    else
    {
        // 개별 컴포넌트 업데이트

        // Last move
        if (ui_render_flags_is_set(flags, RENDER_LAST_MOVE))
        {
            Position last_move = board_get_last_move(board);
            if (ui->prev_last_row != last_move.row || ui->prev_last_col != last_move.col)
            {
                game_info_ui_draw_last_move(win, board);
                ui->prev_last_row = last_move.row;
                ui->prev_last_col = last_move.col;
                need_refresh = true;
            }
            ui_render_flags_clear(flags, RENDER_LAST_MOVE);
        }

        // Current turn
        if (ui_render_flags_is_set(flags, RENDER_CURRENT_TURN))
        {
            Stone current_player = turn_manager_get_current_player(turn_mgr);
            if (ui->prev_current_player != current_player)
            {
                game_info_ui_draw_current_turn(win, current_player);
                ui->prev_current_player = current_player;
                need_refresh = true;
            }
            ui_render_flags_clear(flags, RENDER_CURRENT_TURN);
        }

        // Timer (초 단위로만)
        if (ui_render_flags_is_set(flags, RENDER_TIMER))
        {
            int remaining = turn_manager_get_remaining_time(turn_mgr);
            if (ui->prev_remaining_seconds != remaining)
            {
                game_info_ui_draw_timer(win, remaining);
                ui->prev_remaining_seconds = remaining;
                need_refresh = true;
            }
            ui_render_flags_clear(flags, RENDER_TIMER);
        }

        // Play time (초 단위로만)
        if (ui_render_flags_is_set(flags, RENDER_PLAY_TIME))
        {
            int elapsed = game_info_ui_get_elapsed_seconds(ui);
            if (ui->prev_elapsed_seconds != elapsed)
            {
                game_info_ui_draw_play_time(win, ui);
                ui->prev_elapsed_seconds = elapsed;
                need_refresh = true;
            }
            ui_render_flags_clear(flags, RENDER_PLAY_TIME);
        }
    }

    if (need_refresh)
    {
        wrefresh(win);
    }
}

// ============================================
// 상단 Info 영역 텍스트 표시 함수 (stdscr에 직접 그림)
// ============================================

// 상대방 이름 표시 - 위치 (49, 1)
void game_info_draw_opponent_name(const char *name)
{
    // 영역 클리어 (10칸)
    move(1, 49);
    for (int i = 0; i < 10; i++)
        addch(' ');
    // 이름 표시
    mvaddstr(1, 50, name);
    refresh();
}

// 뷰어 수 표시 - 위치 (60, 1) - 형식: "{n} of Viewers"
void game_info_draw_viewers(int count)
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
void game_info_draw_ping(int ping_ms)
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
void game_info_draw_port(int port)
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
