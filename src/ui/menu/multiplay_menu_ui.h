#ifndef MULTIPLAY_MENU_UI_H
#define MULTIPLAY_MENU_UI_H

#include <ncurses.h>
#include <stdbool.h>
#include "../../game/core/board.h"

#define MAX_IP_LENGTH 16
#define MAX_PORT_LENGTH 6
#define MAX_NAME_LENGTH 9

// HOST/JOIN 선택 UI
typedef enum
{
    MP_MODE_HOST = 0,
    MP_MODE_JOIN = 1
} MultiplayMode;

typedef struct
{
    int selected;      // 0: Host, 1: Join
    int option_count;  // 2
    bool needs_render; // 렌더링 필요 여부
} MultiplayModeSelectUI;

// 연결 정보 입력 UI (IP + PORT + Name 한 화면)
typedef enum
{
    INPUT_FIELD_IP = 0,
    INPUT_FIELD_PORT = 1,
    INPUT_FIELD_NAME = 2
} ConnectionInputField;

typedef struct
{
    char ip_address[MAX_IP_LENGTH];
    char port[MAX_PORT_LENGTH];
    char name[MAX_NAME_LENGTH];
    int ip_cursor;                     // IP 입력 커서 위치
    int port_cursor;                   // PORT 입력 커서 위치
    int name_cursor;                   // Name 입력 커서 위치
    ConnectionInputField active_field; // 현재 활성 필드 (0: IP, 1: PORT, 2: Name)
    bool needs_render;                 // 렌더링 필요 여부
} ConnectionInputUI;

// HOST 설정 입력 UI (Name만)
typedef struct
{
    char name[MAX_NAME_LENGTH];
    int name_cursor;
    bool needs_render; // 렌더링 필요 여부
} HostSettingsUI;

// 규칙 선택 UI (호스트용)
typedef struct
{
    int selected;      // 0: Standard, 1: Renju
    int option_count;  // 2
    bool needs_render; // 렌더링 필요 여부
} MultiplayRuleSelectUI;

// 관전자 연결 정보 입력 UI (IP + PORT + Name)
typedef enum
{
    SPECTATOR_FIELD_IP = 0,
    SPECTATOR_FIELD_PORT = 1,
    SPECTATOR_FIELD_NAME = 2
} SpectatorInputField;

typedef struct
{
    char ip_address[MAX_IP_LENGTH];
    char port[MAX_PORT_LENGTH];
    char name[MAX_NAME_LENGTH];
    int ip_cursor;
    int port_cursor;
    int name_cursor;
    SpectatorInputField active_field;
    bool needs_render; // 렌더링 필요 여부
} SpectatorInputUI;

// 연결 대기 UI 상태
typedef enum
{
    CONNECT_STATE_WAITING = 0,
    CONNECT_STATE_SUCCESS = 1,
    CONNECT_STATE_FAILED = 2,
    CONNECT_STATE_CANCELLED = 3
} ConnectionState;

typedef struct
{
    char ip_address[MAX_IP_LENGTH];
    int port;
    char player_name[MAX_NAME_LENGTH];
    int elapsed_seconds;    // 경과 시간
    int timeout_seconds;    // 타임아웃 (기본 10초)
    ConnectionState state;  // 현재 연결 상태
    char error_message[64]; // 오류 메시지
    bool is_spectator;      // 관전자 모드 여부
} ConnectionWaitUI;

// HOST 대기 UI
typedef struct
{
    char local_ip[MAX_IP_LENGTH];
    int port;
    char host_name[MAX_NAME_LENGTH];
    int elapsed_seconds;                    // 경과 시간
    int anim_frame;                         // 원형 애니메이션 프레임 (0-3, 0.25초)
    int dot_frame;                          // 점 애니메이션 프레임 (0-3, 0.5초)
    ConnectionState state;                  // 현재 연결 상태
    char connected_client[MAX_NAME_LENGTH]; // 연결된 클라이언트 이름
} HostWaitUI;

// ==========================================
// HOST/JOIN 선택 UI 함수
// ==========================================
void multiplay_mode_select_ui_init(MultiplayModeSelectUI *ui);
void multiplay_mode_select_ui_render(WINDOW *win, MultiplayModeSelectUI *ui);
void multiplay_mode_select_ui_move(MultiplayModeSelectUI *ui, int direction);
MultiplayMode multiplay_mode_select_ui_get_selected(const MultiplayModeSelectUI *ui);

// ==========================================
// 연결 정보 입력 UI 함수 (JOIN/CLIENT용)
// ==========================================
void connection_input_ui_init(ConnectionInputUI *ui);
void connection_input_ui_render(WINDOW *win, ConnectionInputUI *ui);
void connection_input_ui_move_field(ConnectionInputUI *ui, int direction);
void connection_input_ui_handle_char(ConnectionInputUI *ui, char ch);
void connection_input_ui_handle_backspace(ConnectionInputUI *ui);
bool connection_input_ui_is_valid(const ConnectionInputUI *ui);
int connection_input_ui_get_port(const ConnectionInputUI *ui);
const char *connection_input_ui_get_ip(const ConnectionInputUI *ui);
const char *connection_input_ui_get_name(const ConnectionInputUI *ui);

// ==========================================
// HOST 설정 입력 UI 함수
// ==========================================
void host_settings_ui_init(HostSettingsUI *ui);
void host_settings_ui_render(WINDOW *win, HostSettingsUI *ui);
void host_settings_ui_handle_char(HostSettingsUI *ui, char ch);
void host_settings_ui_handle_backspace(HostSettingsUI *ui);
bool host_settings_ui_is_valid(const HostSettingsUI *ui);
const char *host_settings_ui_get_name(const HostSettingsUI *ui);

// ==========================================
// 규칙 선택 UI 함수 (HOST용)
// ==========================================
void multiplay_rule_select_ui_init(MultiplayRuleSelectUI *ui);
void multiplay_rule_select_ui_render(WINDOW *win, MultiplayRuleSelectUI *ui);
void multiplay_rule_select_ui_move(MultiplayRuleSelectUI *ui, int direction);
GameRule multiplay_rule_select_ui_get_selected(const MultiplayRuleSelectUI *ui);

// ==========================================
// 관전자 입력 UI 함수
// ==========================================
void spectator_input_ui_init(SpectatorInputUI *ui);
void spectator_input_ui_render(WINDOW *win, SpectatorInputUI *ui);
void spectator_input_ui_move_field(SpectatorInputUI *ui, int direction);
void spectator_input_ui_handle_char(SpectatorInputUI *ui, char ch);
void spectator_input_ui_handle_backspace(SpectatorInputUI *ui);
bool spectator_input_ui_is_valid(const SpectatorInputUI *ui);
int spectator_input_ui_get_port(const SpectatorInputUI *ui);
const char *spectator_input_ui_get_ip(const SpectatorInputUI *ui);
const char *spectator_input_ui_get_name(const SpectatorInputUI *ui);

// ==========================================
// 연결 대기 UI 함수
// ==========================================
void connection_wait_ui_init(ConnectionWaitUI *ui, const char *ip, int port, const char *name, bool is_spectator);
void connection_wait_ui_render(WINDOW *win, const ConnectionWaitUI *ui);
void connection_fail_ui_render(WINDOW *win, const ConnectionWaitUI *ui);

// 연결 대기 화면 실행 (10초 타임아웃, 반환: 0=성공, -1=실패/취소)
int multiplay_wait_connection(const char *ip, int port, const char *name);
int spectator_wait_connection(const char *ip, int port, const char *name);

// ==========================================
// HOST 대기 UI 함수
// ==========================================
void host_wait_ui_init(HostWaitUI *ui, const char *local_ip, int port, const char *host_name);
void host_wait_ui_render(WINDOW *win, const HostWaitUI *ui);
void host_wait_ui_render_dynamic(WINDOW *win, const HostWaitUI *ui);

// HOST 대기 화면 실행 (클라이언트 연결 대기, 반환: 0=성공, -1=취소)
int multiplay_host_wait_for_client(int port, const char *host_name, GameRule rule);

// ==========================================
// 통합 실행 함수
// ==========================================

// HOST/JOIN 선택 화면 실행 (반환: 0=Host, 1=Join, -1=취소)
int multiplay_select_mode(void);

// 연결 정보 입력 화면 실행 (반환: 0=성공, -1=취소)
int multiplay_input_connection(char *ip_out, int *port_out, char *name_out);

// HOST 설정 입력 화면 실행 (반환: 0=성공, -1=취소)
int multiplay_input_host_settings(char *name_out);

// 규칙 선택 화면 실행 (반환: 0=Standard, 1=Renju, -1=취소)
int multiplay_select_rule(void);

// 관전자 연결 정보 입력 화면 실행 (반환: 0=성공, -1=취소)
int spectator_input_connection(char *ip_out, int *port_out, char *name_out);

#endif // MULTIPLAY_MENU_UI_H
