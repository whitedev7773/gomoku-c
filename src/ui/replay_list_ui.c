#include "replay_list_ui.h"
#include <string.h>

// 초기화
bool replay_list_ui_init(ReplayListUI *ui) {
    if (!ui) return false;

    memset(ui, 0, sizeof(ReplayListUI));

    if (!replay_get_log_files(&ui->file_list)) {
        return false;
    }

    ui->selected_index = 0;
    ui->scroll_offset = 0;

    return ui->file_list.file_count > 0;
}

// 렌더링
void replay_list_ui_render(WINDOW *win, const ReplayListUI *ui) {
    if (!win || !ui) return;

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    werase(win);
    box(win, 0, 0);

    mvwprintw(win, 0, 2, " Select Replay File ");

    if (ui->file_list.file_count == 0) {
        mvwprintw(win, max_y / 2, (max_x - 20) / 2, "No replay files found");
        wrefresh(win);
        return;
    }

    // 표시 가능한 줄 수
    int visible_lines = max_y - 4;
    int start_index = ui->scroll_offset;
    int end_index = start_index + visible_lines;

    if (end_index > ui->file_list.file_count) {
        end_index = ui->file_list.file_count;
    }

    // 파일 목록 렌더링
    int y = 2;
    for (int i = start_index; i < end_index; i++) {
        const LogFileInfo *info = &ui->file_list.files[i];

        if (i == ui->selected_index) {
            wattron(win, A_REVERSE);
            mvwprintw(win, y, 2, "> %s", info->display_name);
            wattroff(win, A_REVERSE);
        } else {
            mvwprintw(win, y, 2, "  %s", info->display_name);
        }

        y++;
    }

    // 스크롤 인디케이터
    if (ui->file_list.file_count > visible_lines) {
        mvwprintw(win, max_y - 2, max_x - 20, "(%d/%d files)",
                  ui->selected_index + 1, ui->file_list.file_count);
    }

    // 조작 안내
    mvwprintw(win, max_y - 1, 2, "UP/DOWN: Select | ENTER: Play | Q: Back");

    wrefresh(win);
}

// 선택 이동
void replay_list_ui_move_selection(ReplayListUI *ui, int delta) {
    if (!ui || ui->file_list.file_count == 0) return;

    ui->selected_index += delta;

    // 범위 체크
    if (ui->selected_index < 0) {
        ui->selected_index = 0;
    } else if (ui->selected_index >= ui->file_list.file_count) {
        ui->selected_index = ui->file_list.file_count - 1;
    }

    // 스크롤 조정
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    int visible_lines = max_y - 4;

    if (ui->selected_index < ui->scroll_offset) {
        ui->scroll_offset = ui->selected_index;
    } else if (ui->selected_index >= ui->scroll_offset + visible_lines) {
        ui->scroll_offset = ui->selected_index - visible_lines + 1;
    }
}

// 선택된 파일 가져오기
const char* replay_list_ui_get_selected_file(const ReplayListUI *ui) {
    if (!ui || ui->selected_index < 0 || ui->selected_index >= ui->file_list.file_count) {
        return NULL;
    }

    return ui->file_list.files[ui->selected_index].filename;
}
