#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <ncurses.h>
#include <stdbool.h>

// Window dimensions based on 100x30 layout (ingame_border.c 기준)
#define UI_MIN_WIDTH 100
#define UI_MIN_HEIGHT 30

// Layout constants - ingame_border.c 레이아웃과 일치하도록 수정
// Board 영역: (0,0) ~ (48,24) - 테두리 내부
#define BOARD_WINDOW_WIDTH 47
#define BOARD_WINDOW_HEIGHT 23
#define BOARD_WINDOW_X 1
#define BOARD_WINDOW_Y 1

// Info 영역 (상단 우측): (49,1) - 테두리 내부
#define INFO_WINDOW_WIDTH 50
#define INFO_WINDOW_HEIGHT 1
#define INFO_WINDOW_X 49
#define INFO_WINDOW_Y 1

// Chat 영역: (49,3) ~ (98,21) - 테두리 내부
#define CHAT_WINDOW_WIDTH 50
#define CHAT_WINDOW_HEIGHT 18
#define CHAT_WINDOW_X 49
#define CHAT_WINDOW_Y 3

// Chat Input 영역: (49,23) - 테두리 내부
#define CHAT_INPUT_WIDTH 50
#define CHAT_INPUT_HEIGHT 1
#define CHAT_INPUT_X 49
#define CHAT_INPUT_Y 23

// Bottom 영역: (1,25) ~ (98,29) - 테두리 내부
#define BOTTOM_WINDOW_WIDTH 98
#define BOTTOM_WINDOW_HEIGHT 5
#define BOTTOM_WINDOW_X 1
#define BOTTOM_WINDOW_Y 25

// ============================================
// Dirty Flag 기반 선택적 렌더링 시스템
// ============================================

// 렌더링 플래그 비트마스크
#define RENDER_NONE 0x0000
#define RENDER_BOARD_CELL 0x0001    // 보드의 특정 셀만 업데이트
#define RENDER_BOARD_CURSOR 0x0002  // 커서 위치 변경
#define RENDER_BOARD_FULL 0x0004    // 보드 전체 (초기화 시)
#define RENDER_TIMER 0x0008         // 턴 타이머
#define RENDER_PLAY_TIME 0x0010     // 플레이 시간
#define RENDER_LAST_MOVE 0x0020     // 마지막 수 표시
#define RENDER_CURRENT_TURN 0x0040  // 현재 턴 표시
#define RENDER_CHAT 0x0080          // 채팅 메시지 영역
#define RENDER_CHAT_INPUT 0x0100    // 채팅 입력창
#define RENDER_LOG 0x0200           // 시스템 로그
#define RENDER_INFO 0x0400          // 상단 정보 영역
#define RENDER_MODAL 0x0800         // 모달 창
#define RENDER_BOTTOM_BORDER 0x1000 // 하단 테두리 (초기화 시)

// 모든 렌더링 플래그
#define RENDER_ALL 0xFFFF

// 렌더링 플래그 구조체
typedef struct
{
    unsigned int flags;      // 현재 dirty 플래그
    int dirty_cells[361][2]; // 변경된 셀 좌표 (row, col) - 최대 19x19
    int dirty_cell_count;    // 변경된 셀 개수
} UIRenderFlags;

typedef struct
{
    WINDOW *board_win;
    WINDOW *info_win;
    WINDOW *chat_win;
    WINDOW *chat_input_win;
    WINDOW *bottom_win;
    bool initialized;
    bool first_render;          // 첫 렌더링 여부 (테두리 등 초기화용)
    UIRenderFlags render_flags; // 렌더링 플래그
} UIManager;

bool ui_manager_init(UIManager *manager);

void ui_manager_cleanup(UIManager *manager);

void ui_manager_refresh_all(UIManager *manager);

void ui_manager_clear_all(UIManager *manager);

bool ui_manager_check_terminal_size(void);

// ============================================
// Dirty Flag 관리 함수
// ============================================

// 렌더링 플래그 초기화
void ui_render_flags_init(UIRenderFlags *flags);

// 특정 플래그 설정 (dirty로 마킹)
void ui_render_flags_set(UIRenderFlags *flags, unsigned int flag);

// 특정 플래그 해제 (clean으로 마킹)
void ui_render_flags_clear(UIRenderFlags *flags, unsigned int flag);

// 모든 플래그 해제
void ui_render_flags_clear_all(UIRenderFlags *flags);

// 특정 플래그 체크
bool ui_render_flags_is_set(const UIRenderFlags *flags, unsigned int flag);

// 변경된 셀 추가 (보드용)
void ui_render_flags_add_dirty_cell(UIRenderFlags *flags, int row, int col);

// 변경된 셀 목록 초기화
void ui_render_flags_clear_dirty_cells(UIRenderFlags *flags);

// 모든 플래그를 dirty로 설정 (초기화/강제 재렌더링용)
void ui_render_flags_set_all(UIRenderFlags *flags);

#endif // UI_MANAGER_H
