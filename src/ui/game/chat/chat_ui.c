#include "chat_ui.h"
#include <string.h>
#include <ctype.h>

// 사용 가능한 명령어
static const char *COMMANDS[] = {
    "/quit",
    "/undo",
    "/giveup",
    "/swap",
    NULL};

void chat_init(ChatUI *chat)
{
    memset(chat, 0, sizeof(ChatUI));
    chat->input_mode = false;
    chat->messages_dirty = true; // 초기에는 전체 렌더링 필요
    chat->input_dirty = true;
    chat->prev_message_count = 0;
}

void chat_add_msg(ChatUI *chat, const char *message, ChatMessageType type)
{
    if (chat->message_count >= MAX_CHAT_MESSAGES)
    {
        // 가장 오래된 메시지 삭제 (FIFO)
        for (int i = 0; i < MAX_CHAT_MESSAGES - 1; i++)
        {
            chat->messages[i] = chat->messages[i + 1];
        }
        chat->message_count = MAX_CHAT_MESSAGES - 1;
    }

    UIChatMessage *msg = &chat->messages[chat->message_count];
    strncpy(msg->message, message, MAX_CHAT_MESSAGE_LENGTH);
    msg->message[MAX_CHAT_MESSAGE_LENGTH] = '\0';
    msg->type = type;
    msg->timestamp = time(NULL);

    chat->message_count++;
    chat->messages_dirty = true; // 메시지 추가됨
}

void chat_render(WINDOW *win, const ChatUI *chat)
{
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    // 채팅 메시지 영역 테두리
    box(win, 0, 0);
    mvwprintw(win, 0, 2, " Chat ");

    // 메시지 렌더링 (말풍선 형태)
    int y = 1;
    int visible_count = (max_y - 4); // 테두리, 타이틀, 입력창 제외
    int start_idx = (chat->message_count > visible_count) ? (chat->message_count - visible_count) : 0;

    for (int i = start_idx; i < chat->message_count && y < max_y - 3; i++)
    {
        const UIChatMessage *msg = &chat->messages[i];

        if (msg->type == CHAT_MSG_SYSTEM)
        {
            // 시스템 메시지: 중앙 정렬, 회색
            wattron(win, COLOR_PAIR(5));
            int x = (max_x - strlen(msg->message)) / 2;
            mvwprintw(win, y, x, "%s", msg->message);
            wattroff(win, COLOR_PAIR(5));
            y++;
        }
        else if (msg->type == CHAT_MSG_USER)
        {
            // 내 메시지: 오른쪽 정렬, 초록색 말풍선
            int msg_len = strlen(msg->message);
            int x = max_x - msg_len - 3;

            wattron(win, COLOR_PAIR(4));
            mvwprintw(win, y, x - 1, "[");
            mvwprintw(win, y, x, "%s", msg->message);
            mvwprintw(win, y, x + msg_len, "]");
            wattroff(win, COLOR_PAIR(4));
            y++;
        }
        else
        {
            // 상대방 메시지: 왼쪽 정렬, 파란색 말풍선
            wattron(win, COLOR_PAIR(6));
            mvwprintw(win, y, 2, "[");
            mvwprintw(win, y, 3, "%s", msg->message);
            mvwprintw(win, y, 3 + strlen(msg->message), "]");
            wattroff(win, COLOR_PAIR(6));
            y++;
        }
    }
}

void chat_render_input(WINDOW *win, const ChatUI *chat, int y, int x)
{
    if (!chat->input_mode)
    {
        wattron(win, COLOR_PAIR(5));
        mvwprintw(win, y, x, "Press Enter or T to chat");
        wattroff(win, COLOR_PAIR(5));
        return;
    }

    // 입력 프롬프트
    mvwprintw(win, y, x, "> ");

    // 입력된 텍스트
    wattron(win, COLOR_PAIR(1));
    mvwprintw(win, y, x + 2, "%s", chat->input_buffer);
    wattroff(win, COLOR_PAIR(1));

    // 자동완성 힌트 (회색으로 표시)
    if (strlen(chat->autocomplete_hint) > 0)
    {
        wattron(win, COLOR_PAIR(8) | A_DIM);
        mvwprintw(win, y, x + 2 + strlen(chat->input_buffer), "%s", chat->autocomplete_hint);
        wattroff(win, COLOR_PAIR(8) | A_DIM);
    }

    // 커서 표시
    wmove(win, y, x + 2 + chat->input_cursor_pos);
}

void chat_enter_input(ChatUI *chat)
{
    chat->input_mode = true;
    memset(chat->input_buffer, 0, sizeof(chat->input_buffer));
    chat->input_cursor_pos = 0;
    memset(chat->autocomplete_hint, 0, sizeof(chat->autocomplete_hint));
    chat->input_dirty = true;
}

void chat_exit_input(ChatUI *chat)
{
    chat->input_mode = false;
    memset(chat->input_buffer, 0, sizeof(chat->input_buffer));
    chat->input_cursor_pos = 0;
    memset(chat->autocomplete_hint, 0, sizeof(chat->autocomplete_hint));
    chat->input_dirty = true;
}

bool chat_is_input_mode(const ChatUI *chat)
{
    return chat->input_mode;
}

void chat_update_autocomplete(ChatUI *chat)
{
    memset(chat->autocomplete_hint, 0, sizeof(chat->autocomplete_hint));

    // 빈 입력이면 힌트 없음
    if (strlen(chat->input_buffer) == 0)
    {
        return;
    }

    // '/'로 시작하면 명령어 자동완성
    if (chat->input_buffer[0] == '/')
    {
        for (int i = 0; COMMANDS[i] != NULL; i++)
        {
            // 입력된 텍스트가 명령어의 접두사인지 확인
            if (strncmp(chat->input_buffer, COMMANDS[i], strlen(chat->input_buffer)) == 0)
            {
                // 나머지 부분을 힌트로 표시
                strcpy(chat->autocomplete_hint, COMMANDS[i] + strlen(chat->input_buffer));
                break;
            }
        }
    }
}

void chat_handle_input(ChatUI *chat, int ch)
{
    if (!chat->input_mode)
    {
        return;
    }

    int len = strlen(chat->input_buffer);

    if (ch == '\n' || ch == KEY_ENTER)
    {
        // Enter: 메시지 전송 (chat_is_ready에서 처리)
        return;
    }
    else if (ch == 27)
    {
        // ESC: 입력 취소
        chat_exit_input(chat);
    }
    else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8)
    {
        // Backspace
        if (chat->input_cursor_pos > 0)
        {
            // 커서 위치의 이전 문자 삭제
            memmove(chat->input_buffer + chat->input_cursor_pos - 1,
                    chat->input_buffer + chat->input_cursor_pos,
                    len - chat->input_cursor_pos + 1);
            chat->input_cursor_pos--;
            chat_update_autocomplete(chat);
            chat->input_dirty = true;
        }
    }
    else if (ch == KEY_LEFT)
    {
        // 왼쪽 화살표
        if (chat->input_cursor_pos > 0)
        {
            chat->input_cursor_pos--;
            chat->input_dirty = true;
        }
    }
    else if (ch == KEY_RIGHT)
    {
        // 오른쪽 화살표
        if (chat->input_cursor_pos < len)
        {
            chat->input_cursor_pos++;
            chat->input_dirty = true;
        }
    }
    else if (ch == '\t')
    {
        // Tab: 자동완성 적용
        if (strlen(chat->autocomplete_hint) > 0)
        {
            strcat(chat->input_buffer, chat->autocomplete_hint);
            chat->input_cursor_pos = strlen(chat->input_buffer);
            memset(chat->autocomplete_hint, 0, sizeof(chat->autocomplete_hint));
            chat->input_dirty = true;
        }
    }
    else if (isprint(ch) && len < MAX_CHAT_INPUT_LENGTH)
    {
        // 일반 문자 입력
        // 커서 위치에 문자 삽입
        memmove(chat->input_buffer + chat->input_cursor_pos + 1,
                chat->input_buffer + chat->input_cursor_pos,
                len - chat->input_cursor_pos + 1);
        chat->input_buffer[chat->input_cursor_pos] = ch;
        chat->input_cursor_pos++;
        chat_update_autocomplete(chat);
        chat->input_dirty = true;
    }
}

bool chat_is_ready(const ChatUI *chat)
{
    return chat->input_mode && strlen(chat->input_buffer) > 0;
}

const char *chat_get_msg(ChatUI *chat)
{
    return chat->input_buffer;
}

void chat_clear_input(ChatUI *chat)
{
    memset(chat->input_buffer, 0, sizeof(chat->input_buffer));
    chat->input_cursor_pos = 0;
    memset(chat->autocomplete_hint, 0, sizeof(chat->autocomplete_hint));
    chat->input_dirty = true;
}

// ============================================
// 선택적 렌더링 함수 (Dirty Flag 기반)
// ============================================

void chat_selective_render(WINDOW *win, ChatUI *chat,
                           UIRenderFlags *flags, bool first_render)
{
    if (!win || !chat)
        return;

    // 첫 렌더링이거나 메시지가 변경됨
    if (first_render || ui_render_flags_is_set(flags, RENDER_CHAT) || chat->messages_dirty)
    {
        chat_render(win, chat);
        wrefresh(win);
        chat->messages_dirty = false;
        chat->prev_message_count = chat->message_count;
        ui_render_flags_clear(flags, RENDER_CHAT);
    }
}

void chat_selective_render_input(WINDOW *win, ChatUI *chat,
                                 UIRenderFlags *flags, bool first_render,
                                 int y, int x)
{
    if (!win || !chat)
        return;

    // 첫 렌더링이거나 입력이 변경됨
    if (first_render || ui_render_flags_is_set(flags, RENDER_CHAT_INPUT) || chat->input_dirty)
    {
        // 입력창 영역만 클리어
        for (int i = 0; i < 3; i++)
        {
            wmove(win, i, 0);
            wclrtoeol(win);
        }
        chat_render_input(win, chat, y, x);
        wrefresh(win);
        chat->input_dirty = false;
        ui_render_flags_clear(flags, RENDER_CHAT_INPUT);
    }
}
