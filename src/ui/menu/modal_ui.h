#ifndef MODAL_UI_H
#define MODAL_UI_H

#include <ncurses.h>
#include <stdbool.h>
#include "../core/input_handler.h"

#define MODAL_MAX_MESSAGE_LENGTH 128

// 모달 타입
typedef enum
{
    MODAL_NONE = 0,
    MODAL_GIVEUP,          // 기권 확인
    MODAL_UNDO_REQUEST,    // 무르기 요청
    MODAL_UNDO_RESPONSE,   // 무르기 응답
    MODAL_GAME_RESULT,     // 게임 결과
    MODAL_SWAP_REQUEST,    // Swap 요청
    MODAL_SWAP_RESPONSE,   // Swap 응답
    MODAL_ERROR,           // 에러 메시지
    MODAL_TERMINAL_WARNING // 터미널 크기 경고 (일시정지)
} ModalType;

// 모달 버튼
typedef enum
{
    BUTTON_OK = 0,
    BUTTON_CANCEL,
    BUTTON_ACCEPT,
    BUTTON_DECLINE,
    BUTTON_YES,
    BUTTON_NO
} ModalButton;

// 모달 결과
typedef enum
{
    MODAL_RESULT_NONE = 0,
    MODAL_RESULT_OK,
    MODAL_RESULT_CANCEL,
    MODAL_RESULT_ACCEPT,
    MODAL_RESULT_DECLINE,
    MODAL_RESULT_YES,
    MODAL_RESULT_NO
} ModalResult;

// 모달 UI 상태
typedef struct
{
    ModalType type;
    char message[MODAL_MAX_MESSAGE_LENGTH];
    bool active;
    int selected_button;
    int button_count;
    ModalButton buttons[3]; // 최대 3개 버튼
} ModalUI;

// 초기화
void modal_ui_init(ModalUI *modal);

// 모달 표시
void modal_ui_show(ModalUI *modal, ModalType type, const char *message);

// 모달 닫기
void modal_ui_close(ModalUI *modal);

// 모달 활성화 여부
bool modal_ui_is_active(const ModalUI *modal);

// 모달 렌더링
void modal_ui_render(WINDOW *parent_win, const ModalUI *modal);

// 모달 입력 처리 (키보드)
ModalResult modal_ui_handle_input(ModalUI *modal, int ch);

// 모달 입력 처리 (게임패드 지원)
ModalResult modal_ui_handle_action(ModalUI *modal, InputAction action);

// 버튼 이름 가져오기
const char *modal_ui_get_button_name(ModalButton button);

#endif // MODAL_UI_H
