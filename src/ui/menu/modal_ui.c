#include "modal_ui.h"
#include "../core/theme.h"
#include <string.h>
#include <stdlib.h>

// 초기화
void modal_ui_init(ModalUI *modal)
{
    if (!modal)
        return;

    memset(modal, 0, sizeof(ModalUI));
    modal->type = MODAL_NONE;
    modal->active = false;
    modal->selected_button = 0;
    modal->button_count = 0;
}

// 모달 버튼 설정
static void modal_ui_setup_buttons(ModalUI *modal)
{
    modal->button_count = 0;

    switch (modal->type)
    {
    case MODAL_GIVEUP:
        modal->buttons[0] = BUTTON_YES;
        modal->buttons[1] = BUTTON_NO;
        modal->button_count = 2;
        modal->selected_button = 1; // 기본값: No
        break;

    case MODAL_GIVEUP_REQUEST:
        // 기권 요청은 표시만 (상대방 응답 대기)
        modal->button_count = 0;
        break;

    case MODAL_GIVEUP_RESPONSE:
        modal->buttons[0] = BUTTON_ACCEPT;
        modal->buttons[1] = BUTTON_DECLINE;
        modal->button_count = 2;
        modal->selected_button = 0;
        break;

    case MODAL_UNDO_REQUEST:
        // 무르기 요청은 표시만 (상대방 응답 대기)
        modal->button_count = 0;
        break;

    case MODAL_UNDO_RESPONSE:
        modal->buttons[0] = BUTTON_ACCEPT;
        modal->buttons[1] = BUTTON_DECLINE;
        modal->button_count = 2;
        modal->selected_button = 0;
        break;

    case MODAL_SWAP_REQUEST:
        // Swap 요청은 표시만
        modal->button_count = 0;
        break;

    case MODAL_SWAP_RESPONSE:
        modal->buttons[0] = BUTTON_ACCEPT;
        modal->buttons[1] = BUTTON_DECLINE;
        modal->button_count = 2;
        modal->selected_button = 0;
        break;

    case MODAL_GAME_RESULT:
    case MODAL_ERROR:
    case MODAL_TERMINAL_WARNING:
        modal->buttons[0] = BUTTON_OK;
        modal->button_count = 1;
        modal->selected_button = 0;
        break;

    default:
        modal->button_count = 0;
        break;
    }
}

// 모달 표시
void modal_ui_show(ModalUI *modal, ModalType type, const char *message)
{
    if (!modal)
        return;

    modal->type = type;
    modal->active = true;

    if (message)
    {
        strncpy(modal->message, message, MODAL_MAX_MESSAGE_LENGTH - 1);
        modal->message[MODAL_MAX_MESSAGE_LENGTH - 1] = '\0';
    }
    else
    {
        modal->message[0] = '\0';
    }

    modal_ui_setup_buttons(modal);
}

// 모달 닫기
void modal_ui_close(ModalUI *modal)
{
    if (!modal)
        return;

    modal->active = false;
    modal->type = MODAL_NONE;
    modal->message[0] = '\0';
}

// 모달 활성화 여부
bool modal_ui_is_active(const ModalUI *modal)
{
    return modal && modal->active;
}

// 버튼 이름 가져오기
const char *modal_ui_get_button_name(ModalButton button)
{
    switch (button)
    {
    case BUTTON_OK:
        return "OK";
    case BUTTON_CANCEL:
        return "Cancel";
    case BUTTON_ACCEPT:
        return "Accept";
    case BUTTON_DECLINE:
        return "Decline";
    case BUTTON_YES:
        return "Yes";
    case BUTTON_NO:
        return "No";
    default:
        return "Unknown";
    }
}

// 모달 렌더링
void modal_ui_render(WINDOW *parent_win, const ModalUI *modal)
{
    if (!parent_win || !modal || !modal->active)
        return;

    int parent_h, parent_w;
    getmaxyx(parent_win, parent_h, parent_w);

    // 모달 크기
    int modal_w = 60;
    int modal_h = 10;
    int modal_y = (parent_h - modal_h) / 2;
    int modal_x = (parent_w - modal_w) / 2;

    // 모달 윤도우 생성
    WINDOW *modal_win = newwin(modal_h, modal_w, modal_y, modal_x);

    // 배경 그리기 (검정색 배경)
    wbkgd(modal_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    werase(modal_win);

    // 테두리 그리기 - 테마 주색상
    wattron(modal_win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    box(modal_win, 0, 0);

    // 타이틀
    const char *title = NULL;
    switch (modal->type)
    {
    case MODAL_GIVEUP:
        title = " Confirm Giveup ";
        break;
    case MODAL_GIVEUP_REQUEST:
        title = " Giveup Request ";
        break;
    case MODAL_GIVEUP_RESPONSE:
        title = " Giveup Request ";
        break;
    case MODAL_UNDO_REQUEST:
        title = " Undo Request ";
        break;
    case MODAL_UNDO_RESPONSE:
        title = " Undo Request ";
        break;
    case MODAL_SWAP_REQUEST:
        title = " Swap Request ";
        break;
    case MODAL_SWAP_RESPONSE:
        title = " Swap Request ";
        break;
    case MODAL_GAME_RESULT:
        title = " Game Result ";
        break;
    case MODAL_ERROR:
        title = " Error ";
        break;
    default:
        title = " Message ";
        break;
    }

    if (title)
    {
        mvwprintw(modal_win, 0, (modal_w - strlen(title)) / 2, "%s", title);
    }
    wattroff(modal_win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 메시지 표시 (중앙 정렬, 여러 줄 지원) - 테마 주색상
    wattron(modal_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    int msg_y = 3;
    int msg_x = 4;
    int max_line_width = modal_w - 8;

    // 간단한 줄바꿈 처리
    char *msg_copy = strdup(modal->message);
    char *line = msg_copy;
    int line_count = 0;

    while (line && *line && line_count < 3)
    {
        int line_len = strlen(line);
        if (line_len > max_line_width)
        {
            // 긴 줄은 잘라서 표시
            char temp = line[max_line_width];
            line[max_line_width] = '\0';
            mvwprintw(modal_win, msg_y + line_count, msg_x, "%s", line);
            line[max_line_width] = temp;
            line += max_line_width;
        }
        else
        {
            mvwprintw(modal_win, msg_y + line_count, msg_x, "%s", line);
            break;
        }
        line_count++;
    }
    free(msg_copy);
    wattroff(modal_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 버튼 렌더링 - 테마 색상 적용
    if (modal->button_count > 0)
    {
        int button_y = modal_h - 3;
        int total_button_width = 0;

        // 전체 버튼 너비 계산
        for (int i = 0; i < modal->button_count; i++)
        {
            total_button_width += strlen(modal_ui_get_button_name(modal->buttons[i])) + 4;
            if (i < modal->button_count - 1)
            {
                total_button_width += 4; // 간격
            }
        }

        int button_x = (modal_w - total_button_width) / 2;

        // 버튼 그리기
        for (int i = 0; i < modal->button_count; i++)
        {
            const char *btn_name = modal_ui_get_button_name(modal->buttons[i]);
            int btn_width = strlen(btn_name) + 4;

            if (i == modal->selected_button)
            {
                wattron(modal_win, A_REVERSE | A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
                mvwprintw(modal_win, button_y, button_x, "[ %s ]", btn_name);
                wattroff(modal_win, A_REVERSE | A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
            }
            else
            {
                wattron(modal_win, COLOR_PAIR(COLOR_PAIR_DIM));
                mvwprintw(modal_win, button_y, button_x, "[ %s ]", btn_name);
                wattroff(modal_win, COLOR_PAIR(COLOR_PAIR_DIM));
            }

            button_x += btn_width + 4;
        }
    }
    else
    {
        // 버튼이 없으면 "Waiting..." 표시 - 테마 INFO 색상
        wattron(modal_win, COLOR_PAIR(COLOR_PAIR_INFO));
        mvwprintw(modal_win, modal_h - 3, (modal_w - 13) / 2, "Waiting...");
        wattroff(modal_win, COLOR_PAIR(COLOR_PAIR_INFO));
    }

    wrefresh(modal_win);
    delwin(modal_win);
}

// 모달 입력 처리 (게임패드 지원)
ModalResult modal_ui_handle_action(ModalUI *modal, InputAction action)
{
    if (!modal || !modal->active)
    {
        return MODAL_RESULT_NONE;
    }

    // 버튼이 없으면 입력 무시 (대기 중)
    if (modal->button_count == 0)
    {
        return MODAL_RESULT_NONE;
    }

    switch (action)
    {
    case INPUT_MOVE_LEFT:
        if (modal->button_count > 1)
        {
            modal->selected_button = (modal->selected_button - 1 + modal->button_count) % modal->button_count;
        }
        return MODAL_RESULT_NONE;

    case INPUT_MOVE_RIGHT:
        if (modal->button_count > 1)
        {
            modal->selected_button = (modal->selected_button + 1) % modal->button_count;
        }
        return MODAL_RESULT_NONE;

    case INPUT_PLACE_STONE: // A 버튼 = 확인
        switch (modal->buttons[modal->selected_button])
        {
        case BUTTON_OK:
            return MODAL_RESULT_OK;
        case BUTTON_CANCEL:
            return MODAL_RESULT_CANCEL;
        case BUTTON_ACCEPT:
            return MODAL_RESULT_ACCEPT;
        case BUTTON_DECLINE:
            return MODAL_RESULT_DECLINE;
        case BUTTON_YES:
            return MODAL_RESULT_YES;
        case BUTTON_NO:
            return MODAL_RESULT_NO;
        default:
            return MODAL_RESULT_NONE;
        }

    case INPUT_QUIT: // B 버튼 = 취소
        return MODAL_RESULT_CANCEL;

    default:
        return MODAL_RESULT_NONE;
    }
}
