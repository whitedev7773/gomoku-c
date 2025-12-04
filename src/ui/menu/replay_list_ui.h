#ifndef REPLAY_LIST_UI_H
#define REPLAY_LIST_UI_H

#include "../../game/feature/replay.h"
#include <ncurses.h>
#include <stdbool.h>

// 리플레이 목록 UI 상태
typedef struct
{
    LogFileList file_list;
    int selected_index;
    int scroll_offset;
} ReplayListUI;

// 초기화
bool replay_list_ui_init(ReplayListUI *ui);

// 렌더링
void replay_list_ui_render(WINDOW *win, const ReplayListUI *ui);

// 선택 이동
void replay_list_ui_move_selection(ReplayListUI *ui, int delta);

// 선택된 파일 가져오기
const char *replay_list_ui_get_selected_file(const ReplayListUI *ui);

#endif // REPLAY_LIST_UI_H
