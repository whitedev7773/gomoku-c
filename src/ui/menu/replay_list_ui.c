#include "replay_list_ui.h"
#include "../core/theme.h"
#include <string.h>

// 초기화
bool replay_list_ui_init(ReplayListUI *ui)
{
    if (!ui)
        return false;

    memset(ui, 0, sizeof(ReplayListUI));

    if (!replay_get_log_files(&ui->file_list))
    {
        return false;
    }

    ui->selected_index = 0;
    ui->scroll_offset = 0;

    return ui->file_list.file_count > 0;
}

// 렌더링
void replay_list_ui_render(WINDOW *win, const ReplayListUI *ui)
{
    if (!win || !ui)
        return;

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    werase(win);

    // 테마 주색상으로 border 렌더링
    wattron(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    box(win, 0, 0);
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 테마 주색상으로 타이틀
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(win, 0, 2, " Select Replay File ");
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    if (ui->file_list.file_count == 0)
    {
        // 빈 폴더 아이콘 - 테마 주색상 적용
        int center_y = max_y / 2 - 4;
        int center_x = (max_x - 24) / 2;

        wattron(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
        mvwprintw(win, center_y, center_x, "    ╭─────────────╮    ");
        mvwprintw(win, center_y + 1, center_x, "   ╭╯             ╰╮   ");
        mvwprintw(win, center_y + 2, center_x, "   │               │   ");
        mvwprintw(win, center_y + 3, center_x, "   │      📂      │   ");
        mvwprintw(win, center_y + 4, center_x, "   │               │   ");
        mvwprintw(win, center_y + 5, center_x, "   ╰───────────────╯   ");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));

        // 메시지 - 테마 INFO 색상
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_INFO));
        mvwprintw(win, center_y + 7, (max_x - 22) / 2, "No Replay Files Found");
        wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_INFO));

        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, center_y + 9, (max_x - 38) / 2, "Play a game first to create replays!");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));

        // 하단 안내
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, max_y - 2, (max_x - 16) / 2, "Press Q to back");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));

        wrefresh(win);
        return;
    }

    // 표시 가능한 줄 수
    int visible_lines = max_y - 4;
    int start_index = ui->scroll_offset;
    int end_index = start_index + visible_lines;

    if (end_index > ui->file_list.file_count)
    {
        end_index = ui->file_list.file_count;
    }

    // 파일 목록 렌더링
    int y = 2;
    for (int i = start_index; i < end_index; i++)
    {
        const LogFileInfo *info = &ui->file_list.files[i];

        if (i == ui->selected_index)
        {
            // 선택 항목 - 테마 주색상 + 반전
            wattron(win, A_BOLD | A_REVERSE | COLOR_PAIR(COLOR_PAIR_DEFAULT));
            mvwprintw(win, y, 2, "> %-*s", max_x - 6, info->display_name);
            wattroff(win, A_BOLD | A_REVERSE | COLOR_PAIR(COLOR_PAIR_DEFAULT));
        }
        else
        {
            wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
            mvwprintw(win, y, 2, "  %s", info->display_name);
            wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
        }

        y++;
    }

    // 스크롤 인디케이터 - 테마 INFO 색상
    if (ui->file_list.file_count > visible_lines)
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_INFO));
        mvwprintw(win, max_y - 2, max_x - 20, "(%d/%d files)",
                  ui->selected_index + 1, ui->file_list.file_count);
        wattroff(win, COLOR_PAIR(COLOR_PAIR_INFO));
    }

    // 조작 안내 - 테마 DIM 색상
    wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
    mvwprintw(win, max_y - 1, 2, "UP/DOWN: Select | ENTER: Play | Q: Back");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));

    wrefresh(win);
}

// 선택 변경만 렌더링 (최적화 - 이전/현재 선택만 업데이트)
void replay_list_ui_render_selection_only(WINDOW *win, const ReplayListUI *ui, int prev_selected)
{
    if (!win || !ui || ui->file_list.file_count == 0)
        return;

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    int visible_lines = max_y - 4;
    int start_index = ui->scroll_offset;
    int end_index = start_index + visible_lines;
    if (end_index > ui->file_list.file_count)
    {
        end_index = ui->file_list.file_count;
    }

    // 이전 선택 항목 클리어 (화면에 보이는 경우만) - 테마 DIM 색상
    if (prev_selected >= start_index && prev_selected < end_index)
    {
        int prev_y = 2 + (prev_selected - start_index);
        const LogFileInfo *prev_info = &ui->file_list.files[prev_selected];
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, prev_y, 2, "  %-*s", max_x - 6, prev_info->display_name);
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
    }

    // 현재 선택 항목 강조 (화면에 보이는 경우만) - 테마 주색상 + 반전
    if (ui->selected_index >= start_index && ui->selected_index < end_index)
    {
        int curr_y = 2 + (ui->selected_index - start_index);
        const LogFileInfo *curr_info = &ui->file_list.files[ui->selected_index];
        wattron(win, A_BOLD | A_REVERSE | COLOR_PAIR(COLOR_PAIR_DEFAULT));
        mvwprintw(win, curr_y, 2, "> %-*s", max_x - 6, curr_info->display_name);
        wattroff(win, A_BOLD | A_REVERSE | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    }

    // 스크롤 인디케이터 업데이트 - 테마 INFO 색상
    if (ui->file_list.file_count > visible_lines)
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_INFO));
        mvwprintw(win, max_y - 2, max_x - 20, "(%d/%d files)",
                  ui->selected_index + 1, ui->file_list.file_count);
        wattroff(win, COLOR_PAIR(COLOR_PAIR_INFO));
    }

    wrefresh(win);
}

// 선택 이동 (스크롤 발생 시 true 반환)
bool replay_list_ui_move_selection(ReplayListUI *ui, int delta)
{
    if (!ui || ui->file_list.file_count == 0)
        return false;

    int old_scroll = ui->scroll_offset;
    ui->selected_index += delta;

    // 범위 체크
    if (ui->selected_index < 0)
    {
        ui->selected_index = 0;
    }
    else if (ui->selected_index >= ui->file_list.file_count)
    {
        ui->selected_index = ui->file_list.file_count - 1;
    }

    // 스크롤 조정
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    int visible_lines = max_y - 4;

    if (ui->selected_index < ui->scroll_offset)
    {
        ui->scroll_offset = ui->selected_index;
    }
    else if (ui->selected_index >= ui->scroll_offset + visible_lines)
    {
        ui->scroll_offset = ui->selected_index - visible_lines + 1;
    }

    // 스크롤 발생 여부 반환
    return (old_scroll != ui->scroll_offset);
}

// 선택된 파일 가져오기
const char *replay_list_ui_get_selected_file(const ReplayListUI *ui)
{
    if (!ui || ui->selected_index < 0 || ui->selected_index >= ui->file_list.file_count)
    {
        return NULL;
    }

    return ui->file_list.files[ui->selected_index].filename;
}
