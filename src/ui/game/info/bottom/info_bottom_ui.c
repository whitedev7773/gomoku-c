#include "info_bottom_ui.h"
#include "../../board/board_ui.h"
#include "../../../../game/core/turn_manager.h"
#include <string.h>
#include <stdio.h>

void info_btm_init(InfoBottomUI *ui)
{
    if (!ui)
        return;

    // 각 하위 모듈 초기화
    last_move_display_init(&ui->last_move);
    now_turn_display_init(&ui->now_turn);
    time_display_init(&ui->time_display);
    system_log_display_init(&ui->system_log);
}

int info_btm_elapsed_sec(const TimeDisplay *time_display)
{
    return time_display_elapsed_sec(time_display);
}

void info_btm_draw_last_mv(WINDOW *win, const Board *board)
{
    last_move_display_draw(win, board);
}

void info_btm_draw_turn(WINDOW *win, Stone current_player)
{
    now_turn_display_draw(win, current_player);
}

void info_btm_draw_timer(WINDOW *win, int remaining_seconds)
{
    time_display_draw_timer(win, remaining_seconds);
}

void info_btm_draw_playtime(WINDOW *win, const TimeDisplay *time_display)
{
    time_display_draw_playtime(win, time_display);
}

void info_btm_draw_btns(WINDOW *win)
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
void info_btm_init_win(WINDOW *win)
{
    if (!win)
        return;

    wclear(win);
    // 테두리는 ingame_border.c에서 stdscr에 그림
    // 여기서는 내용만 그림
    wrefresh(win);
}

// 최적화된 업데이트 (변경된 부분만) - 분리된 모듈 사용
void info_btm_update(WINDOW *win, const Board *board,
                     const TurnManager *turn_mgr,
                     InfoBottomUI *ui)
{
    if (!win || !board || !turn_mgr || !ui)
        return;

    bool need_refresh = false;

    // 1. Last move 변경 체크
    Position last_move = board_get_last_move(board);
    if (ui->last_move.prev_row != last_move.row || ui->last_move.prev_col != last_move.col)
    {
        last_move_display_draw(win, board);
        ui->last_move.prev_row = last_move.row;
        ui->last_move.prev_col = last_move.col;
        need_refresh = true;
    }

    // 2. 현재 턴 변경 체크
    Stone current_player = turn_manager_get_current_player(turn_mgr);
    if (ui->now_turn.prev_current_player != current_player)
    {
        now_turn_display_draw(win, current_player);
        ui->now_turn.prev_current_player = current_player;
        need_refresh = true;
    }

    // 3. 타이머는 초 단위로만 업데이트
    int remaining = turn_manager_get_remaining_time(turn_mgr);
    if (ui->time_display.prev_remaining_seconds != remaining)
    {
        time_display_draw_timer(win, remaining);
        ui->time_display.prev_remaining_seconds = remaining;
        need_refresh = true;
    }

    // 4. 경과 시간 (초 단위로만 체크)
    int elapsed = time_display_elapsed_sec(&ui->time_display);
    if (ui->time_display.prev_elapsed_seconds != elapsed)
    {
        time_display_draw_playtime(win, &ui->time_display);
        ui->time_display.prev_elapsed_seconds = elapsed;
        need_refresh = true;
    }

    // 5. 시스템 로그 업데이트
    if (ui->system_log.dirty)
    {
        system_log_display_draw(win, &ui->system_log);
        ui->system_log.dirty = false;
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

void info_btm_render(WINDOW *win, const Board *board,
                     const TurnManager *turn_mgr,
                     InfoBottomUI *ui,
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
        info_btm_draw_btns(win);

        // 모든 정보 그리기 (분리된 모듈 사용)
        last_move_display_draw(win, board);
        now_turn_display_draw(win, turn_manager_get_current_player(turn_mgr));
        time_display_draw_timer(win, turn_manager_get_remaining_time(turn_mgr));
        time_display_draw_playtime(win, &ui->time_display);
        system_log_display_draw(win, &ui->system_log);

        // 상태 갱신 (분리된 모듈의 상태 업데이트)
        Position last_move = board_get_last_move(board);
        ui->last_move.prev_row = last_move.row;
        ui->last_move.prev_col = last_move.col;
        ui->now_turn.prev_current_player = turn_manager_get_current_player(turn_mgr);
        ui->time_display.prev_remaining_seconds = turn_manager_get_remaining_time(turn_mgr);
        ui->time_display.prev_elapsed_seconds = time_display_elapsed_sec(&ui->time_display);
        ui->system_log.dirty = false;

        ui_render_flags_clear(flags, RENDER_BOTTOM_BORDER);
        need_refresh = true;
    }
    else
    {
        // 개별 컴포넌트 업데이트 (분리된 모듈의 render 함수 사용)

        // Last move
        if (last_move_display_render(win, board, &ui->last_move, flags, false))
        {
            need_refresh = true;
        }

        // Current turn
        if (now_turn_display_render(win, turn_mgr, &ui->now_turn, flags, false))
        {
            need_refresh = true;
        }

        // Timer
        if (time_display_render_timer(win, turn_mgr, &ui->time_display, flags, false))
        {
            need_refresh = true;
        }

        // Play time
        if (time_display_render_playtime(win, &ui->time_display, flags, false))
        {
            need_refresh = true;
        }

        // System log
        if (system_log_display_render(win, &ui->system_log, flags, false))
        {
            need_refresh = true;
        }
    }

    if (need_refresh)
    {
        wrefresh(win);
    }
}

// 시스템 로그 추가 편의 함수
void info_btm_add_system_log(InfoBottomUI *ui, const char *message)
{
    if (!ui || !message)
        return;
    system_log_display_add(&ui->system_log, message);
}