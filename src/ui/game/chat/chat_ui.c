#include "chat_ui.h"
#include "../../core/theme.h"
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
    chat_add_msg_with_time(chat, message, type, 0);
}

void chat_add_msg_with_time(ChatUI *chat, const char *message, ChatMessageType type, int game_time_seconds)
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
    msg->game_time_seconds = game_time_seconds;

    chat->message_count++;
    chat->messages_dirty = true; // 메시지 추가됨
}

void chat_render(WINDOW *win, const ChatUI *chat)
{
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    // 채팅 영역 내부 클리어
    for (int i = 1; i < max_y - 1; i++)
    {
        wmove(win, i, 1);
        for (int j = 1; j < max_x - 1; j++)
            waddch(win, ' ');
    }

    // 메시지 렌더링 (버블 형태 - 아래에서 위로)
    // 각 메시지는 3줄 사용: 상단 테두리, 내용, 하단 테두리
    int visible_lines = max_y - 2; // 테두리 제외
    int lines_per_msg = 3;
    int max_visible_msgs = visible_lines / lines_per_msg;

    int start_idx = (chat->message_count > max_visible_msgs)
                        ? (chat->message_count - max_visible_msgs)
                        : 0;

    int y = 1;
    for (int i = start_idx; i < chat->message_count && y + lines_per_msg <= max_y - 1; i++)
    {
        const UIChatMessage *msg = &chat->messages[i];
        int msg_len = strlen(msg->message);

        // 시간 문자열 생성 (MM:SS)
        char time_str[8];
        int mins = msg->game_time_seconds / 60;
        int secs = msg->game_time_seconds % 60;
        snprintf(time_str, sizeof(time_str), "%02d:%02d", mins, secs);

        if (msg->type == CHAT_MSG_SYSTEM)
        {
            // 시스템 메시지: 중앙 정렬, 회색, 1줄만 사용
            wattron(win, COLOR_PAIR(COLOR_PAIR_SYSTEM));
            int x = (max_x - msg_len) / 2;
            if (x < 1)
                x = 1;
            mvwprintw(win, y, x, "%s", msg->message);
            wattroff(win, COLOR_PAIR(COLOR_PAIR_SYSTEM));
            y += 1;
        }
        else if (msg->type == CHAT_MSG_USER)
        {
            // 내 메시지 (ME): 오른쪽 정렬
            // 형식:                       ┌──────────────┐ ──
            //                       00:06 │ Hello World! │ ME
            //                             └──────────────┘ ──
            int bubble_width = msg_len + 2;                     // 내용 + 좌우 여백
            int total_width = 6 + 1 + bubble_width + 1 + 2 + 3; // time + space + bubble + space + ME + padding
            int start_x = max_x - total_width - 1;
            if (start_x < 1)
                start_x = 1;

            // 상단 테두리
            wattron(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE));
            mvwprintw(win, y, start_x + 7, "\u250c");
            for (int j = 0; j < msg_len; j++)
                waddstr(win, "\u2500");
            waddstr(win, "\u2510");
            mvwprintw(win, y, start_x + 7 + msg_len + 2 + 1, "\u2500\u2500");
            wattroff(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE));

            // 중간 (시간 + 내용 + ME)
            y++;
            wattron(win, COLOR_PAIR(COLOR_PAIR_INFO));
            mvwprintw(win, y, start_x, "%s", time_str);
            wattroff(win, COLOR_PAIR(COLOR_PAIR_INFO));

            wattron(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE));
            mvwprintw(win, y, start_x + 6, " \u2502 %s \u2502", msg->message);
            wattroff(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE));

            wattron(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_BOLD);
            mvwprintw(win, y, start_x + 6 + 3 + msg_len + 3, "ME");
            wattroff(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_BOLD);

            // 하단 테두리
            y++;
            wattron(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE));
            mvwprintw(win, y, start_x + 7, "\u2514");
            for (int j = 0; j < msg_len; j++)
                waddstr(win, "\u2500");
            waddstr(win, "\u2518");
            mvwprintw(win, y, start_x + 7 + msg_len + 2 + 1, "\u2500\u2500");
            wattroff(win, COLOR_PAIR(COLOR_PAIR_WHITE_STONE));
            y++;
        }
        else
        {
            // 상대방 메시지 (OP): 왼쪽 정렬
            // 형식: ── ┌───────────────────┐
            //       OP │ Is this Online??? │ 00:09
            //       ── └───────────────────┘

            // 상단 테두리
            wattron(win, COLOR_PAIR(COLOR_PAIR_OPPONENT));
            mvwprintw(win, y, 1, "\u2500\u2500 \u250c");
            for (int j = 0; j < msg_len; j++)
                waddstr(win, "\u2500");
            waddstr(win, "\u2510");
            wattroff(win, COLOR_PAIR(COLOR_PAIR_OPPONENT));

            // 중간 (OP + 내용 + 시간)
            y++;
            wattron(win, COLOR_PAIR(COLOR_PAIR_OPPONENT) | A_BOLD);
            mvwprintw(win, y, 1, "OP");
            wattroff(win, COLOR_PAIR(COLOR_PAIR_OPPONENT) | A_BOLD);

            wattron(win, COLOR_PAIR(COLOR_PAIR_OPPONENT));
            mvwprintw(win, y, 4, "\u2502 %s \u2502", msg->message);
            wattroff(win, COLOR_PAIR(COLOR_PAIR_OPPONENT));

            wattron(win, COLOR_PAIR(COLOR_PAIR_INFO));
            mvwprintw(win, y, 4 + 2 + msg_len + 2 + 1, "%s", time_str);
            wattroff(win, COLOR_PAIR(COLOR_PAIR_INFO));

            // 하단 테두리
            y++;
            wattron(win, COLOR_PAIR(COLOR_PAIR_OPPONENT));
            mvwprintw(win, y, 1, "\u2500\u2500 \u2514");
            for (int j = 0; j < msg_len; j++)
                waddstr(win, "\u2500");
            waddstr(win, "\u2518");
            wattroff(win, COLOR_PAIR(COLOR_PAIR_OPPONENT));
            y++;
        }
    }
}

void chat_render_input(WINDOW *win, const ChatUI *chat, int y, int x)
{
    int input_len = strlen(chat->input_buffer);

    if (!chat->input_mode)
    {
        // 입력 모드 아닐 때: "Enter to Chat..." 플레이스홀더
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, y, x, "Enter to Chat...");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));

        // 글자수 표시 (0/34)
        mvwprintw(win, y, x + 35, " %2d/%d", input_len, MAX_CHAT_INPUT_LENGTH);
        return;
    }

    // 입력 모드: 입력된 텍스트 표시
    wattron(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    // 입력 영역 클리어 (34칸)
    mvwprintw(win, y, x, "                                  ");
    mvwprintw(win, y, x, "%s", chat->input_buffer);
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 자동완성 힌트 (회색으로 표시)
    if (strlen(chat->autocomplete_hint) > 0)
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM) | A_DIM);
        mvwprintw(win, y, x + input_len, "%s", chat->autocomplete_hint);
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM) | A_DIM);
    }

    // 글자수 표시 (n/34)
    if (input_len >= MAX_CHAT_INPUT_LENGTH - 5)
    {
        // 글자수가 거의 찼으면 경고 색상
        wattron(win, COLOR_PAIR(COLOR_PAIR_SYSTEM) | A_BOLD);
    }
    mvwprintw(win, y, x + 35, " %2d/%d", input_len, MAX_CHAT_INPUT_LENGTH);
    if (input_len >= MAX_CHAT_INPUT_LENGTH - 5)
    {
        wattroff(win, COLOR_PAIR(COLOR_PAIR_SYSTEM) | A_BOLD);
    }

    // 커서 표시
    wmove(win, y, x + chat->input_cursor_pos);
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
