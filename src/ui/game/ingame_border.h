#ifndef INGAME_BORDER_H
#define INGAME_BORDER_H

#include <ncurses.h>
#include <stdbool.h>

// 인게임 전체 Border를 그리는 모듈
// Border는 게임 중 한번만 그리고 Modal이 뜰 때만 재렌더링

// Border 초기화 및 그리기
// stdscr에 전체 인게임 레이아웃의 테두리를 그림
void ingame_border_draw(void);

// 특정 영역만 Border 재그리기 (Modal 닫힌 후 등)
void ingame_border_redraw_board_area(void);
void ingame_border_redraw_info_area(void);
void ingame_border_redraw_chat_area(void);
void ingame_border_redraw_bottom_area(void);

// 전체 Border 재그리기 (Modal 닫힌 후 전체 복원)
void ingame_border_redraw_all(void);

#endif // INGAME_BORDER_H
