#ifndef CHAT_UI_H
#define CHAT_UI_H

#include <ncurses.h>
#include <stdbool.h>
#include <time.h>
#include "../../core/ui_manager.h"

#define MAX_CHAT_MESSAGES 10
#define MAX_CHAT_MESSAGE_LENGTH 30
#define MAX_CHAT_INPUT_LENGTH 30

// 채팅 메시지 타입
typedef enum
{
    CHAT_MSG_USER,     // 내가 보낸 메시지
    CHAT_MSG_OPPONENT, // 상대방이 보낸 메시지
    CHAT_MSG_SYSTEM    // 시스템 메시지
} ChatMessageType;

// 채팅 메시지 (UI용)
typedef struct
{
    char message[MAX_CHAT_MESSAGE_LENGTH + 1];
    ChatMessageType type;
    time_t timestamp;
    int game_time_seconds; // 게임 시간 (초) - MM:SS 표시용
} UIChatMessage;

// 채팅 UI 상태
typedef struct
{
    UIChatMessage messages[MAX_CHAT_MESSAGES];
    int message_count;
    int scroll_offset;

    bool input_mode;
    char input_buffer[MAX_CHAT_INPUT_LENGTH + 1];
    int input_cursor_pos;

    char autocomplete_hint[MAX_CHAT_MESSAGE_LENGTH + 1];

    // Dirty flag 관련
    bool messages_dirty;    // 메시지 목록이 변경됨
    bool input_dirty;       // 입력창이 변경됨
    int prev_message_count; // 이전 메시지 수
} ChatUI;

// 초기화
void chat_init(ChatUI *chat);

// 메시지 추가
void chat_add_msg(ChatUI *chat, const char *message, ChatMessageType type);
void chat_add_msg_with_time(ChatUI *chat, const char *message, ChatMessageType type, int game_time_seconds);

// 렌더링
void chat_render(WINDOW *win, const ChatUI *chat);
void chat_render_input(WINDOW *win, const ChatUI *chat, int y, int x);

// 입력 모드
void chat_enter_input(ChatUI *chat);
void chat_exit_input(ChatUI *chat);
bool chat_is_input_mode(const ChatUI *chat);

// 입력 처리
void chat_handle_input(ChatUI *chat, int ch);
bool chat_is_ready(const ChatUI *chat);
const char *chat_get_msg(ChatUI *chat);
void chat_clear_input(ChatUI *chat);

// 자동완성
void chat_update_autocomplete(ChatUI *chat);

// ============================================
// 선택적 렌더링 함수 (Dirty Flag 기반)
// ============================================

// 선택적 렌더링 (dirty flag 기반)
void chat_selective_render(WINDOW *win, ChatUI *chat,
                           UIRenderFlags *flags, bool first_render);
void chat_selective_render_input(WINDOW *win, ChatUI *chat,
                                 UIRenderFlags *flags, bool first_render,
                                 int y, int x);

#endif // CHAT_UI_H
