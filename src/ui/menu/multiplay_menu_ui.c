#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "multiplay_menu_ui.h"
#include "../core/input_handler.h"
#include "../core/theme.h"
#include "../core/ui_manager.h"
#include "../../network/core/network.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>

// ==========================================
// HOST/JOIN 선택 UI
// ==========================================

void multiplay_mode_select_ui_init(MultiplayModeSelectUI *ui)
{
    if (!ui)
        return;

    ui->selected = 0; // Host가 기본값
    ui->option_count = 2;
}

void multiplay_mode_select_ui_render(WINDOW *win, const MultiplayModeSelectUI *ui)
{
    if (!win || !ui)
        return;

    wclear(win);

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    // 외곽 박스
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    box(win, 0, 0);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 타이틀
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(win, 2, (max_x - 26) / 2, "╔═══════════════════════╗");
    mvwprintw(win, 3, (max_x - 26) / 2, "║  MULTIPLAYER MODE     ║");
    mvwprintw(win, 4, (max_x - 26) / 2, "╚═══════════════════════╝");
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 옵션 박스
    int box_y = 8;
    int box_w = 70;
    int box_h = 12;
    int box_x = (max_x - box_w) / 2;

    // 옵션 박스 테두리
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwaddch(win, box_y, box_x, ACS_ULCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y, box_x + box_w - 1, ACS_URCORNER);

    for (int j = 1; j < box_h - 1; j++)
    {
        mvwaddch(win, box_y + j, box_x, ACS_VLINE);
        mvwaddch(win, box_y + j, box_x + box_w - 1, ACS_VLINE);
    }

    mvwaddch(win, box_y + box_h - 1, box_x, ACS_LLCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y + box_h - 1, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y + box_h - 1, box_x + box_w - 1, ACS_LRCORNER);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 옵션들 (가로 정렬)
    int option_y = box_y + 5;
    int option_width = 26;
    int gap = 8;
    int total_width = option_width * 2 + gap;
    int start_x = box_x + (box_w - total_width) / 2;

    // HOST 옵션
    int host_x = start_x;
    if (ui->selected == 0)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_REVERSE);
        mvwprintw(win, option_y - 1, host_x, "╔════════════════════════╗");
        mvwprintw(win, option_y + 0, host_x, "║     ▶ HOST GAME ◀      ║");
        mvwprintw(win, option_y + 1, host_x, "║     Create & Wait      ║");
        mvwprintw(win, option_y + 2, host_x, "╚════════════════════════╝");
        wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_REVERSE);
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, option_y - 1, host_x, "┌────────────────────────┐");
        mvwprintw(win, option_y + 0, host_x, "│       HOST GAME        │");
        mvwprintw(win, option_y + 1, host_x, "│     Create & Wait      │");
        mvwprintw(win, option_y + 2, host_x, "└────────────────────────┘");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
    }

    // JOIN 옵션
    int join_x = start_x + option_width + gap;
    if (ui->selected == 1)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_REVERSE);
        mvwprintw(win, option_y - 1, join_x, "╔════════════════════════╗");
        mvwprintw(win, option_y + 0, join_x, "║     ▶ JOIN  GAME ◀     ║");
        mvwprintw(win, option_y + 1, join_x, "║     Connect to Host    ║");
        mvwprintw(win, option_y + 2, join_x, "╚════════════════════════╝");
        wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_REVERSE);
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, option_y - 1, join_x, "┌────────────────────────┐");
        mvwprintw(win, option_y + 0, join_x, "│       JOIN  GAME       │");
        mvwprintw(win, option_y + 1, join_x, "│     Connect to Host    │");
        mvwprintw(win, option_y + 2, join_x, "└────────────────────────┘");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
    }

    // 설명 텍스트
    wattron(win, COLOR_PAIR(COLOR_PAIR_INFO));
    if (ui->selected == 0)
    {
        mvwprintw(win, box_y + box_h + 1, (max_x - 50) / 2, "  Create a game server and wait for opponent to join  ");
    }
    else
    {
        mvwprintw(win, box_y + box_h + 1, (max_x - 50) / 2, "   Enter server IP and port to connect to a game     ");
    }
    wattroff(win, COLOR_PAIR(COLOR_PAIR_INFO));

    // 안내 메시지
    wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
    mvwprintw(win, max_y - 3, (max_x - 40) / 2, "←→ / ↑↓: Select    ↵: Confirm    Q: Back");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));

    wrefresh(win);
}

void multiplay_mode_select_ui_move(MultiplayModeSelectUI *ui, int direction)
{
    if (!ui)
        return;

    ui->selected += direction;

    // 순환
    if (ui->selected < 0)
        ui->selected = ui->option_count - 1;
    else if (ui->selected >= ui->option_count)
        ui->selected = 0;
}

MultiplayMode multiplay_mode_select_ui_get_selected(const MultiplayModeSelectUI *ui)
{
    if (!ui)
        return MP_MODE_HOST;
    return (ui->selected == 0) ? MP_MODE_HOST : MP_MODE_JOIN;
}

// ==========================================
// 연결 정보 입력 UI (IP + PORT + Name 한 화면)
// ==========================================

void connection_input_ui_init(ConnectionInputUI *ui)
{
    if (!ui)
        return;

    memset(ui->ip_address, 0, MAX_IP_LENGTH);
    strncpy(ui->port, "7773", MAX_PORT_LENGTH - 1); // 기본 포트
    ui->port[MAX_PORT_LENGTH - 1] = '\0';
    strncpy(ui->name, "Player", MAX_NAME_LENGTH - 1); // 기본 이름
    ui->name[MAX_NAME_LENGTH - 1] = '\0';
    ui->ip_cursor = 0;
    ui->port_cursor = 4;
    ui->name_cursor = 6;
    ui->active_field = INPUT_FIELD_IP;
}

void connection_input_ui_render(WINDOW *win, const ConnectionInputUI *ui)
{
    if (!win || !ui)
        return;

    wclear(win);

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    // 외곽 박스
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    box(win, 0, 0);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 타이틀
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(win, 2, (max_x - 26) / 2, "╔═══════════════════════╗");
    mvwprintw(win, 3, (max_x - 26) / 2, "║    JOIN GAME (LAN)    ║");
    mvwprintw(win, 4, (max_x - 26) / 2, "╚═══════════════════════╝");
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 입력 박스
    int box_y = 7;
    int box_w = 60;
    int box_h = 15;
    int box_x = (max_x - box_w) / 2;

    // 입력 박스 테두리
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwaddch(win, box_y, box_x, ACS_ULCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y, box_x + box_w - 1, ACS_URCORNER);

    for (int j = 1; j < box_h - 1; j++)
    {
        mvwaddch(win, box_y + j, box_x, ACS_VLINE);
        mvwaddch(win, box_y + j, box_x + box_w - 1, ACS_VLINE);
    }

    mvwaddch(win, box_y + box_h - 1, box_x, ACS_LLCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y + box_h - 1, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y + box_h - 1, box_x + box_w - 1, ACS_LRCORNER);
    wattroff(win, A_BOLD);

    int label_x = box_x + 6;
    int input_x = box_x + 20;
    int input_w = 30;

    // IP 입력 필드
    int ip_y = box_y + 2;
    bool ip_active = (ui->active_field == INPUT_FIELD_IP);

    if (ip_active)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE));
        mvwprintw(win, ip_y + 1, label_x, "▶ Server IP:");
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, ip_y + 1, label_x, "  Server IP:");
    }

    // IP 입력 박스
    if (ip_active)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
        mvwprintw(win, ip_y, input_x, "┌");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┐");

        mvwprintw(win, ip_y + 1, input_x, "│ %-*s│", input_w - 3, ui->ip_address);

        mvwprintw(win, ip_y + 2, input_x, "└");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┘");
        wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

        // 커서 표시
        mvwaddch(win, ip_y + 1, input_x + 2 + ui->ip_cursor, '_' | A_BLINK);
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, ip_y, input_x, "┌");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┐");

        mvwprintw(win, ip_y + 1, input_x, "│ %-*s│", input_w - 3, ui->ip_address);

        mvwprintw(win, ip_y + 2, input_x, "└");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┘");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
    }

    // PORT 입력 필드
    int port_y = box_y + 6;
    bool port_active = (ui->active_field == INPUT_FIELD_PORT);

    if (port_active)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE));
        mvwprintw(win, port_y + 1, label_x, "▶ Port:     ");
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, port_y + 1, label_x, "  Port:     ");
    }

    // PORT 입력 박스
    if (port_active)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
        mvwprintw(win, port_y, input_x, "┌");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┐");

        mvwprintw(win, port_y + 1, input_x, "│ %-*s│", input_w - 3, ui->port);

        mvwprintw(win, port_y + 2, input_x, "└");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┘");
        wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

        // 커서 표시
        mvwaddch(win, port_y + 1, input_x + 2 + ui->port_cursor, '_' | A_BLINK);
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, port_y, input_x, "┌");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┐");

        mvwprintw(win, port_y + 1, input_x, "│ %-*s│", input_w - 3, ui->port);

        mvwprintw(win, port_y + 2, input_x, "└");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┘");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
    }

    // NAME 입력 필드
    int name_y = box_y + 10;
    bool name_active = (ui->active_field == INPUT_FIELD_NAME);

    if (name_active)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE));
        mvwprintw(win, name_y + 1, label_x, "▶ Nickname: ");
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, name_y + 1, label_x, "  Nickname: ");
    }

    // NAME 입력 박스
    if (name_active)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
        mvwprintw(win, name_y, input_x, "┌");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┐");

        mvwprintw(win, name_y + 1, input_x, "│ %-*s│", input_w - 3, ui->name);

        mvwprintw(win, name_y + 2, input_x, "└");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┘");
        wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

        // 커서 표시
        mvwaddch(win, name_y + 1, input_x + 2 + ui->name_cursor, '_' | A_BLINK);
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, name_y, input_x, "┌");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┐");

        mvwprintw(win, name_y + 1, input_x, "│ %-*s│", input_w - 3, ui->name);

        mvwprintw(win, name_y + 2, input_x, "└");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┘");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
    }

    // 유효성 표시
    wattron(win, COLOR_PAIR(COLOR_PAIR_INFO));
    if (strlen(ui->ip_address) == 0)
    {
        mvwprintw(win, box_y + box_h + 1, (max_x - 42) / 2, "Enter server IP address (e.g. 192.168.0.1)");
    }
    else
    {
        mvwprintw(win, box_y + box_h + 1, (max_x - 42) / 2, "Press Enter to connect                    ");
    }
    wattroff(win, COLOR_PAIR(COLOR_PAIR_INFO));

    // 안내 메시지
    wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
    mvwprintw(win, max_y - 3, (max_x - 50) / 2, "↑↓: Switch Field   Type to Edit   ↵: Connect   Q: Back");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));

    wrefresh(win);
}

void connection_input_ui_move_field(ConnectionInputUI *ui, int direction)
{
    if (!ui)
        return;

    if (direction < 0)
    {
        // 위로
        if (ui->active_field == INPUT_FIELD_PORT)
            ui->active_field = INPUT_FIELD_IP;
        else if (ui->active_field == INPUT_FIELD_NAME)
            ui->active_field = INPUT_FIELD_PORT;
    }
    else
    {
        // 아래로
        if (ui->active_field == INPUT_FIELD_IP)
            ui->active_field = INPUT_FIELD_PORT;
        else if (ui->active_field == INPUT_FIELD_PORT)
            ui->active_field = INPUT_FIELD_NAME;
    }
}

void connection_input_ui_handle_char(ConnectionInputUI *ui, char ch)
{
    if (!ui)
        return;

    if (ui->active_field == INPUT_FIELD_IP)
    {
        // IP 주소: 숫자와 점만 허용
        if ((isdigit(ch) || ch == '.') && ui->ip_cursor < MAX_IP_LENGTH - 1)
        {
            ui->ip_address[ui->ip_cursor++] = ch;
            ui->ip_address[ui->ip_cursor] = '\0';
        }
    }
    else if (ui->active_field == INPUT_FIELD_PORT)
    {
        // PORT: 숫자만 허용
        if (isdigit(ch) && ui->port_cursor < MAX_PORT_LENGTH - 1)
        {
            ui->port[ui->port_cursor++] = ch;
            ui->port[ui->port_cursor] = '\0';
        }
    }
    else
    {
        // NAME: 영문, 숫자만 허용
        if ((isalnum(ch) || ch == '_') && ui->name_cursor < MAX_NAME_LENGTH - 1)
        {
            ui->name[ui->name_cursor++] = ch;
            ui->name[ui->name_cursor] = '\0';
        }
    }
}

void connection_input_ui_handle_backspace(ConnectionInputUI *ui)
{
    if (!ui)
        return;

    if (ui->active_field == INPUT_FIELD_IP)
    {
        if (ui->ip_cursor > 0)
        {
            ui->ip_cursor--;
            ui->ip_address[ui->ip_cursor] = '\0';
        }
    }
    else if (ui->active_field == INPUT_FIELD_PORT)
    {
        if (ui->port_cursor > 0)
        {
            ui->port_cursor--;
            ui->port[ui->port_cursor] = '\0';
        }
    }
    else
    {
        if (ui->name_cursor > 0)
        {
            ui->name_cursor--;
            ui->name[ui->name_cursor] = '\0';
        }
    }
}

bool connection_input_ui_is_valid(const ConnectionInputUI *ui)
{
    if (!ui)
        return false;

    // IP 주소가 비어있으면 무효
    if (strlen(ui->ip_address) == 0)
        return false;

    // 포트가 비어있으면 기본값 사용
    return true;
}

int connection_input_ui_get_port(const ConnectionInputUI *ui)
{
    if (!ui)
        return DEFAULT_PORT;

    if (strlen(ui->port) == 0)
        return DEFAULT_PORT;

    char *endptr;
    long port = strtol(ui->port, &endptr, 10);
    if (*endptr != '\0' || port < 1 || port > 65535)
        return DEFAULT_PORT;

    return (int)port;
}

const char *connection_input_ui_get_ip(const ConnectionInputUI *ui)
{
    if (!ui)
        return "";
    return ui->ip_address;
}

const char *connection_input_ui_get_name(const ConnectionInputUI *ui)
{
    if (!ui)
        return "Player";

    if (strlen(ui->name) == 0)
        return "Player";

    return ui->name;
}

// ==========================================
// HOST 설정 입력 UI (Name만)
// ==========================================

void host_settings_ui_init(HostSettingsUI *ui)
{
    if (!ui)
        return;

    strncpy(ui->name, "Host", MAX_NAME_LENGTH - 1); // 기본 이름
    ui->name[MAX_NAME_LENGTH - 1] = '\0';
    ui->name_cursor = 4;
}

void host_settings_ui_render(WINDOW *win, const HostSettingsUI *ui)
{
    if (!win || !ui)
        return;

    wclear(win);

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    // 외곽 박스
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    box(win, 0, 0);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 타이틀
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(win, 2, (max_x - 26) / 2, "╔═══════════════════════╗");
    mvwprintw(win, 3, (max_x - 26) / 2, "║   HOST GAME (LAN)     ║");
    mvwprintw(win, 4, (max_x - 26) / 2, "╚═══════════════════════╝");
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 입력 박스
    int box_y = 10;
    int box_w = 60;
    int box_h = 7;
    int box_x = (max_x - box_w) / 2;

    // 입력 박스 테두리
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwaddch(win, box_y, box_x, ACS_ULCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y, box_x + box_w - 1, ACS_URCORNER);

    for (int j = 1; j < box_h - 1; j++)
    {
        mvwaddch(win, box_y + j, box_x, ACS_VLINE);
        mvwaddch(win, box_y + j, box_x + box_w - 1, ACS_VLINE);
    }

    mvwaddch(win, box_y + box_h - 1, box_x, ACS_LLCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y + box_h - 1, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y + box_h - 1, box_x + box_w - 1, ACS_LRCORNER);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    int label_x = box_x + 6;
    int input_x = box_x + 20;
    int input_w = 30;

    // NAME 입력 필드
    int name_y = box_y + 2;

    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE));
    mvwprintw(win, name_y + 1, label_x, "▶ Nickname: ");

    // NAME 입력 박스
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(win, name_y, input_x, "┌");
    for (int i = 0; i < input_w - 2; i++)
        wprintw(win, "─");
    wprintw(win, "┐");

    mvwprintw(win, name_y + 1, input_x, "│ %-*s│", input_w - 3, ui->name);

    mvwprintw(win, name_y + 2, input_x, "└");
    for (int i = 0; i < input_w - 2; i++)
        wprintw(win, "─");
    wprintw(win, "┘");
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 커서 표시
    mvwaddch(win, name_y + 1, input_x + 2 + ui->name_cursor, '_' | A_BLINK);

    // 유효성 표시
    wattron(win, COLOR_PAIR(COLOR_PAIR_INFO));
    mvwprintw(win, box_y + box_h + 1, (max_x - 42) / 2, "Enter your nickname (max 8 characters)   ");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_INFO));

    // 안내 메시지
    wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
    mvwprintw(win, max_y - 3, (max_x - 42) / 2, "Type to Edit   ↵: Continue   Q: Back");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));

    wrefresh(win);
}

void host_settings_ui_handle_char(HostSettingsUI *ui, char ch)
{
    if (!ui)
        return;

    // NAME: 영문, 숫자만 허용
    if ((isalnum(ch) || ch == '_') && ui->name_cursor < MAX_NAME_LENGTH - 1)
    {
        ui->name[ui->name_cursor++] = ch;
        ui->name[ui->name_cursor] = '\0';
    }
}

void host_settings_ui_handle_backspace(HostSettingsUI *ui)
{
    if (!ui)
        return;

    if (ui->name_cursor > 0)
    {
        ui->name_cursor--;
        ui->name[ui->name_cursor] = '\0';
    }
}

bool host_settings_ui_is_valid(const HostSettingsUI *ui)
{
    if (!ui)
        return false;

    // 이름이 비어있어도 기본값 사용
    return true;
}

const char *host_settings_ui_get_name(const HostSettingsUI *ui)
{
    if (!ui)
        return "Host";

    if (strlen(ui->name) == 0)
        return "Host";

    return ui->name;
}

// ==========================================
// 규칙 선택 UI (HOST용)
// ==========================================

void multiplay_rule_select_ui_init(MultiplayRuleSelectUI *ui)
{
    if (!ui)
        return;

    ui->selected = 1; // Renju가 기본값
    ui->option_count = 2;
}

void multiplay_rule_select_ui_render(WINDOW *win, const MultiplayRuleSelectUI *ui)
{
    if (!win || !ui)
        return;

    wclear(win);

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    // 외곽 박스
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    box(win, 0, 0);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 타이틀
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(win, 2, (max_x - 26) / 2, "╔═══════════════════════╗");
    mvwprintw(win, 3, (max_x - 26) / 2, "║  SELECT GAME RULE     ║");
    mvwprintw(win, 4, (max_x - 26) / 2, "╚═══════════════════════╝");
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 옵션 박스
    int box_y = 8;
    int box_w = 70;
    int box_h = 12;
    int box_x = (max_x - box_w) / 2;

    // 옵션 박스 테두리
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwaddch(win, box_y, box_x, ACS_ULCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y, box_x + box_w - 1, ACS_URCORNER);

    for (int j = 1; j < box_h - 1; j++)
    {
        mvwaddch(win, box_y + j, box_x, ACS_VLINE);
        mvwaddch(win, box_y + j, box_x + box_w - 1, ACS_VLINE);
    }

    mvwaddch(win, box_y + box_h - 1, box_x, ACS_LLCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y + box_h - 1, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y + box_h - 1, box_x + box_w - 1, ACS_LRCORNER);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 옵션들 (가로 정렬)
    int option_y = box_y + 5;
    int option_width = 26;
    int gap = 8;
    int total_width = option_width * 2 + gap;
    int start_x = box_x + (box_w - total_width) / 2;

    // STANDARD 옵션
    int std_x = start_x;
    if (ui->selected == 0)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_REVERSE);
        mvwprintw(win, option_y - 1, std_x, "╔════════════════════════╗");
        mvwprintw(win, option_y + 0, std_x, "║    ▶ STANDARD ◀        ║");
        mvwprintw(win, option_y + 1, std_x, "║   No Forbidden Moves   ║");
        mvwprintw(win, option_y + 2, std_x, "╚════════════════════════╝");
        wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_REVERSE);
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, option_y - 1, std_x, "┌────────────────────────┐");
        mvwprintw(win, option_y + 0, std_x, "│       STANDARD         │");
        mvwprintw(win, option_y + 1, std_x, "│    No Forbidden Moves  │");
        mvwprintw(win, option_y + 2, std_x, "└────────────────────────┘");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
    }

    // RENJU 옵션
    int renju_x = start_x + option_width + gap;
    if (ui->selected == 1)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_REVERSE);
        mvwprintw(win, option_y - 1, renju_x, "╔════════════════════════╗");
        mvwprintw(win, option_y + 0, renju_x, "║      ▶ RENJU ◀         ║");
        mvwprintw(win, option_y + 1, renju_x, "║   Forbidden for BLACK  ║");
        mvwprintw(win, option_y + 2, renju_x, "╚════════════════════════╝");
        wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE) | A_REVERSE);
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, option_y - 1, renju_x, "┌────────────────────────┐");
        mvwprintw(win, option_y + 0, renju_x, "│        RENJU           │");
        mvwprintw(win, option_y + 1, renju_x, "│    Forbidden for BLACK │");
        mvwprintw(win, option_y + 2, renju_x, "└────────────────────────┘");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
    }

    // 설명 텍스트
    wattron(win, COLOR_PAIR(COLOR_PAIR_INFO));
    if (ui->selected == 0)
    {
        mvwprintw(win, box_y + box_h + 1, (max_x - 48) / 2, "  Classic Gomoku - No forbidden moves for anyone  ");
    }
    else
    {
        mvwprintw(win, box_y + box_h + 1, (max_x - 54) / 2, "Professional Rule - 3-3, 4-4, Overline forbidden for BLACK");
    }
    wattroff(win, COLOR_PAIR(COLOR_PAIR_INFO));

    // 안내 메시지
    wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
    mvwprintw(win, max_y - 3, (max_x - 40) / 2, "←→ / ↑↓: Select    ↵: Confirm    Q: Back");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));

    wrefresh(win);
}

void multiplay_rule_select_ui_move(MultiplayRuleSelectUI *ui, int direction)
{
    if (!ui)
        return;

    ui->selected += direction;

    // 순환
    if (ui->selected < 0)
        ui->selected = ui->option_count - 1;
    else if (ui->selected >= ui->option_count)
        ui->selected = 0;
}

GameRule multiplay_rule_select_ui_get_selected(const MultiplayRuleSelectUI *ui)
{
    if (!ui)
        return RULE_RENJU;
    return (ui->selected == 0) ? RULE_STANDARD : RULE_RENJU;
}

// ==========================================
// 관전자 입력 UI
// ==========================================

void spectator_input_ui_init(SpectatorInputUI *ui)
{
    if (!ui)
        return;

    memset(ui->ip_address, 0, MAX_IP_LENGTH);
    strncpy(ui->port, "7773", MAX_PORT_LENGTH - 1); // 기본 포트
    ui->port[MAX_PORT_LENGTH - 1] = '\0';
    strncpy(ui->name, "Viewer", MAX_NAME_LENGTH - 1); // 기본 이름
    ui->name[MAX_NAME_LENGTH - 1] = '\0';
    ui->ip_cursor = 0;
    ui->port_cursor = 4;
    ui->name_cursor = 6;
    ui->active_field = SPECTATOR_FIELD_IP;
}

void spectator_input_ui_render(WINDOW *win, const SpectatorInputUI *ui)
{
    if (!win || !ui)
        return;

    wclear(win);

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    // 외곽 박스
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    box(win, 0, 0);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 타이틀
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(win, 2, (max_x - 26) / 2, "╔═══════════════════════╗");
    mvwprintw(win, 3, (max_x - 26) / 2, "║   SPECTATOR MODE      ║");
    mvwprintw(win, 4, (max_x - 26) / 2, "╚═══════════════════════╝");
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 입력 박스
    int box_y = 7;
    int box_w = 60;
    int box_h = 15;
    int box_x = (max_x - box_w) / 2;

    // 입력 박스 테두리
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwaddch(win, box_y, box_x, ACS_ULCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y, box_x + box_w - 1, ACS_URCORNER);

    for (int j = 1; j < box_h - 1; j++)
    {
        mvwaddch(win, box_y + j, box_x, ACS_VLINE);
        mvwaddch(win, box_y + j, box_x + box_w - 1, ACS_VLINE);
    }

    mvwaddch(win, box_y + box_h - 1, box_x, ACS_LLCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y + box_h - 1, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y + box_h - 1, box_x + box_w - 1, ACS_LRCORNER);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    int label_x = box_x + 6;
    int input_x = box_x + 20;
    int input_w = 30;

    // IP 입력 필드
    int ip_y = box_y + 2;
    bool ip_active = (ui->active_field == SPECTATOR_FIELD_IP);

    if (ip_active)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE));
        mvwprintw(win, ip_y + 1, label_x, "▶ Server IP:");
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, ip_y + 1, label_x, "  Server IP:");
    }

    // IP 입력 박스
    if (ip_active)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
        mvwprintw(win, ip_y, input_x, "┌");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┐");

        mvwprintw(win, ip_y + 1, input_x, "│ %-*s│", input_w - 3, ui->ip_address);

        mvwprintw(win, ip_y + 2, input_x, "└");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┘");
        wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

        // 커서 표시
        mvwaddch(win, ip_y + 1, input_x + 2 + ui->ip_cursor, '_' | A_BLINK);
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, ip_y, input_x, "┌");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┐");

        mvwprintw(win, ip_y + 1, input_x, "│ %-*s│", input_w - 3, ui->ip_address);

        mvwprintw(win, ip_y + 2, input_x, "└");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┘");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
    }

    // PORT 입력 필드
    int port_y = box_y + 6;
    bool port_active = (ui->active_field == SPECTATOR_FIELD_PORT);

    if (port_active)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE));
        mvwprintw(win, port_y + 1, label_x, "▶ Port:     ");
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, port_y + 1, label_x, "  Port:     ");
    }

    // PORT 입력 박스
    if (port_active)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
        mvwprintw(win, port_y, input_x, "┌");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┐");

        mvwprintw(win, port_y + 1, input_x, "│ %-*s│", input_w - 3, ui->port);

        mvwprintw(win, port_y + 2, input_x, "└");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┘");
        wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

        // 커서 표시
        mvwaddch(win, port_y + 1, input_x + 2 + ui->port_cursor, '_' | A_BLINK);
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, port_y, input_x, "┌");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┐");

        mvwprintw(win, port_y + 1, input_x, "│ %-*s│", input_w - 3, ui->port);

        mvwprintw(win, port_y + 2, input_x, "└");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┘");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
    }

    // NAME 입력 필드
    int name_y = box_y + 10;
    bool name_active = (ui->active_field == SPECTATOR_FIELD_NAME);

    if (name_active)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE));
        mvwprintw(win, name_y + 1, label_x, "▶ Your Name:");
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, name_y + 1, label_x, "  Your Name:");
    }

    // NAME 입력 박스
    if (name_active)
    {
        wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
        mvwprintw(win, name_y, input_x, "┌");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┐");

        mvwprintw(win, name_y + 1, input_x, "│ %-*s│", input_w - 3, ui->name);

        mvwprintw(win, name_y + 2, input_x, "└");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┘");
        wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

        // 커서 표시
        mvwaddch(win, name_y + 1, input_x + 2 + ui->name_cursor, '_' | A_BLINK);
    }
    else
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, name_y, input_x, "┌");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┐");

        mvwprintw(win, name_y + 1, input_x, "│ %-*s│", input_w - 3, ui->name);

        mvwprintw(win, name_y + 2, input_x, "└");
        for (int i = 0; i < input_w - 2; i++)
            wprintw(win, "─");
        wprintw(win, "┘");
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
    }

    // 유효성 표시
    wattron(win, COLOR_PAIR(COLOR_PAIR_INFO));
    if (strlen(ui->ip_address) == 0)
    {
        mvwprintw(win, box_y + box_h + 1, (max_x - 40) / 2, "Enter server IP address (e.g. 192.168.0.1)");
    }
    else
    {
        mvwprintw(win, box_y + box_h + 1, (max_x - 40) / 2, "Press Enter to connect as spectator      ");
    }
    wattroff(win, COLOR_PAIR(COLOR_PAIR_INFO));

    // 안내 메시지
    wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
    mvwprintw(win, max_y - 3, (max_x - 50) / 2, "↑↓: Switch Field   Type to Edit   ↵: Connect   Q: Back");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));

    wrefresh(win);
}

void spectator_input_ui_move_field(SpectatorInputUI *ui, int direction)
{
    if (!ui)
        return;

    if (direction < 0)
    {
        // 위로
        if (ui->active_field == SPECTATOR_FIELD_PORT)
            ui->active_field = SPECTATOR_FIELD_IP;
        else if (ui->active_field == SPECTATOR_FIELD_NAME)
            ui->active_field = SPECTATOR_FIELD_PORT;
    }
    else
    {
        // 아래로
        if (ui->active_field == SPECTATOR_FIELD_IP)
            ui->active_field = SPECTATOR_FIELD_PORT;
        else if (ui->active_field == SPECTATOR_FIELD_PORT)
            ui->active_field = SPECTATOR_FIELD_NAME;
    }
}

void spectator_input_ui_handle_char(SpectatorInputUI *ui, char ch)
{
    if (!ui)
        return;

    if (ui->active_field == SPECTATOR_FIELD_IP)
    {
        // IP 주소: 숫자와 점만 허용
        if ((isdigit(ch) || ch == '.') && ui->ip_cursor < MAX_IP_LENGTH - 1)
        {
            ui->ip_address[ui->ip_cursor++] = ch;
            ui->ip_address[ui->ip_cursor] = '\0';
        }
    }
    else if (ui->active_field == SPECTATOR_FIELD_PORT)
    {
        // PORT: 숫자만 허용
        if (isdigit(ch) && ui->port_cursor < MAX_PORT_LENGTH - 1)
        {
            ui->port[ui->port_cursor++] = ch;
            ui->port[ui->port_cursor] = '\0';
        }
    }
    else
    {
        // NAME: 영문, 숫자만 허용
        if ((isalnum(ch) || ch == '_') && ui->name_cursor < MAX_NAME_LENGTH - 1)
        {
            ui->name[ui->name_cursor++] = ch;
            ui->name[ui->name_cursor] = '\0';
        }
    }
}

void spectator_input_ui_handle_backspace(SpectatorInputUI *ui)
{
    if (!ui)
        return;

    if (ui->active_field == SPECTATOR_FIELD_IP)
    {
        if (ui->ip_cursor > 0)
        {
            ui->ip_cursor--;
            ui->ip_address[ui->ip_cursor] = '\0';
        }
    }
    else if (ui->active_field == SPECTATOR_FIELD_PORT)
    {
        if (ui->port_cursor > 0)
        {
            ui->port_cursor--;
            ui->port[ui->port_cursor] = '\0';
        }
    }
    else
    {
        if (ui->name_cursor > 0)
        {
            ui->name_cursor--;
            ui->name[ui->name_cursor] = '\0';
        }
    }
}

bool spectator_input_ui_is_valid(const SpectatorInputUI *ui)
{
    if (!ui)
        return false;

    // IP 주소가 비어있으면 무효
    if (strlen(ui->ip_address) == 0)
        return false;

    return true;
}

int spectator_input_ui_get_port(const SpectatorInputUI *ui)
{
    if (!ui)
        return DEFAULT_PORT;

    if (strlen(ui->port) == 0)
        return DEFAULT_PORT;

    char *endptr;
    long port = strtol(ui->port, &endptr, 10);
    if (*endptr != '\0' || port < 1 || port > 65535)
        return DEFAULT_PORT;

    return (int)port;
}

const char *spectator_input_ui_get_ip(const SpectatorInputUI *ui)
{
    if (!ui)
        return "";
    return ui->ip_address;
}

const char *spectator_input_ui_get_name(const SpectatorInputUI *ui)
{
    if (!ui)
        return "Viewer";

    if (strlen(ui->name) == 0)
        return "Viewer";

    return ui->name;
}

// ==========================================
// 통합 실행 함수
// ==========================================

int multiplay_select_mode(void)
{
    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    if (has_colors())
    {
        start_color();
        ThemeType saved_theme = theme_load_from_config();
        theme_init(saved_theme);
    }

    wresize(stdscr, UI_MIN_HEIGHT, UI_MIN_WIDTH);
    WINDOW *select_win = newwin(UI_MIN_HEIGHT, UI_MIN_WIDTH, 0, 0);
    keypad(select_win, TRUE);

    InputHandler input_handler;
    input_handler_init(&input_handler);

    MultiplayModeSelectUI ui;
    multiplay_mode_select_ui_init(&ui);

    bool running = true;
    int result = -1;

    while (running)
    {
        multiplay_mode_select_ui_render(select_win, &ui);

        InputEvent event = input_handler_get_event(&input_handler, select_win);

        switch (event.action)
        {
        case INPUT_MOVE_UP:
        case INPUT_MOVE_LEFT:
            multiplay_mode_select_ui_move(&ui, -1);
            break;

        case INPUT_MOVE_DOWN:
        case INPUT_MOVE_RIGHT:
            multiplay_mode_select_ui_move(&ui, 1);
            break;

        case INPUT_PLACE_STONE:
            result = ui.selected;
            running = false;
            break;

        case INPUT_QUIT:
            result = -1;
            running = false;
            break;

        default:
            break;
        }
    }

    input_handler_cleanup(&input_handler);
    delwin(select_win);
    endwin();

    return result;
}

int multiplay_input_connection(char *ip_out, int *port_out, char *name_out)
{
    if (!ip_out || !port_out || !name_out)
        return -1;

    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    if (has_colors())
    {
        start_color();
        ThemeType saved_theme = theme_load_from_config();
        theme_init(saved_theme);
    }

    wresize(stdscr, UI_MIN_HEIGHT, UI_MIN_WIDTH);
    WINDOW *input_win = newwin(UI_MIN_HEIGHT, UI_MIN_WIDTH, 0, 0);
    keypad(input_win, TRUE);

    InputHandler input_handler;
    input_handler_init(&input_handler);

    ConnectionInputUI ui;
    connection_input_ui_init(&ui);

    bool running = true;
    int result = -1;

    while (running)
    {
        connection_input_ui_render(input_win, &ui);

        InputEvent event = input_handler_get_event(&input_handler, input_win);

        switch (event.action)
        {
        case INPUT_MOVE_UP:
            connection_input_ui_move_field(&ui, -1);
            break;

        case INPUT_MOVE_DOWN:
            connection_input_ui_move_field(&ui, 1);
            break;

        case INPUT_PLACE_STONE:
            if (connection_input_ui_is_valid(&ui))
            {
                strncpy(ip_out, connection_input_ui_get_ip(&ui), MAX_IP_LENGTH - 1);
                ip_out[MAX_IP_LENGTH - 1] = '\0';
                *port_out = connection_input_ui_get_port(&ui);
                strncpy(name_out, connection_input_ui_get_name(&ui), MAX_NAME_LENGTH - 1);
                name_out[MAX_NAME_LENGTH - 1] = '\0';
                result = 0;
                running = false;
            }
            break;

        case INPUT_QUIT:
            result = -1;
            running = false;
            break;

        default:
            // 키 코드를 확인하여 문자 입력 처리
            if (event.key_code == KEY_BACKSPACE || event.key_code == 127 || event.key_code == 8)
            {
                connection_input_ui_handle_backspace(&ui);
            }
            else if (event.key_code >= 32 && event.key_code < 127)
            {
                connection_input_ui_handle_char(&ui, (char)event.key_code);
            }
            break;
        }
    }

    input_handler_cleanup(&input_handler);
    delwin(input_win);
    endwin();

    return result;
}

int multiplay_select_rule(void)
{
    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    if (has_colors())
    {
        start_color();
        ThemeType saved_theme = theme_load_from_config();
        theme_init(saved_theme);
    }

    wresize(stdscr, UI_MIN_HEIGHT, UI_MIN_WIDTH);
    WINDOW *select_win = newwin(UI_MIN_HEIGHT, UI_MIN_WIDTH, 0, 0);
    keypad(select_win, TRUE);

    InputHandler input_handler;
    input_handler_init(&input_handler);

    MultiplayRuleSelectUI ui;
    multiplay_rule_select_ui_init(&ui);

    bool running = true;
    int result = -1;

    while (running)
    {
        multiplay_rule_select_ui_render(select_win, &ui);

        InputEvent event = input_handler_get_event(&input_handler, select_win);

        switch (event.action)
        {
        case INPUT_MOVE_UP:
        case INPUT_MOVE_LEFT:
            multiplay_rule_select_ui_move(&ui, -1);
            break;

        case INPUT_MOVE_DOWN:
        case INPUT_MOVE_RIGHT:
            multiplay_rule_select_ui_move(&ui, 1);
            break;

        case INPUT_PLACE_STONE:
            result = ui.selected;
            running = false;
            break;

        case INPUT_QUIT:
            result = -1;
            running = false;
            break;

        default:
            break;
        }
    }

    input_handler_cleanup(&input_handler);
    delwin(select_win);
    endwin();

    return result;
}

int multiplay_input_host_settings(char *name_out)
{
    if (!name_out)
        return -1;

    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    if (has_colors())
    {
        start_color();
        ThemeType saved_theme = theme_load_from_config();
        theme_init(saved_theme);
    }

    wresize(stdscr, UI_MIN_HEIGHT, UI_MIN_WIDTH);
    WINDOW *input_win = newwin(UI_MIN_HEIGHT, UI_MIN_WIDTH, 0, 0);
    keypad(input_win, TRUE);

    InputHandler input_handler;
    input_handler_init(&input_handler);

    HostSettingsUI ui;
    host_settings_ui_init(&ui);

    bool running = true;
    int result = -1;

    while (running)
    {
        host_settings_ui_render(input_win, &ui);

        InputEvent event = input_handler_get_event(&input_handler, input_win);

        switch (event.action)
        {
        case INPUT_PLACE_STONE:
            strncpy(name_out, host_settings_ui_get_name(&ui), MAX_NAME_LENGTH - 1);
            name_out[MAX_NAME_LENGTH - 1] = '\0';
            result = 0;
            running = false;
            break;

        case INPUT_QUIT:
            result = -1;
            running = false;
            break;

        default:
            // 키 코드를 확인하여 문자 입력 처리
            if (event.key_code == KEY_BACKSPACE || event.key_code == 127 || event.key_code == 8)
            {
                host_settings_ui_handle_backspace(&ui);
            }
            else if (event.key_code >= 32 && event.key_code < 127)
            {
                host_settings_ui_handle_char(&ui, (char)event.key_code);
            }
            break;
        }
    }

    input_handler_cleanup(&input_handler);
    delwin(input_win);
    endwin();

    return result;
}

int spectator_input_connection(char *ip_out, int *port_out, char *name_out)
{
    if (!ip_out || !port_out || !name_out)
        return -1;

    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    if (has_colors())
    {
        start_color();
        ThemeType saved_theme = theme_load_from_config();
        theme_init(saved_theme);
    }

    wresize(stdscr, UI_MIN_HEIGHT, UI_MIN_WIDTH);
    WINDOW *input_win = newwin(UI_MIN_HEIGHT, UI_MIN_WIDTH, 0, 0);
    keypad(input_win, TRUE);

    InputHandler input_handler;
    input_handler_init(&input_handler);

    SpectatorInputUI ui;
    spectator_input_ui_init(&ui);

    bool running = true;
    int result = -1;

    while (running)
    {
        spectator_input_ui_render(input_win, &ui);

        InputEvent event = input_handler_get_event(&input_handler, input_win);

        switch (event.action)
        {
        case INPUT_MOVE_UP:
            spectator_input_ui_move_field(&ui, -1);
            break;

        case INPUT_MOVE_DOWN:
            spectator_input_ui_move_field(&ui, 1);
            break;

        case INPUT_PLACE_STONE:
            if (spectator_input_ui_is_valid(&ui))
            {
                strncpy(ip_out, spectator_input_ui_get_ip(&ui), MAX_IP_LENGTH - 1);
                ip_out[MAX_IP_LENGTH - 1] = '\0';
                *port_out = spectator_input_ui_get_port(&ui);
                strncpy(name_out, spectator_input_ui_get_name(&ui), MAX_NAME_LENGTH - 1);
                name_out[MAX_NAME_LENGTH - 1] = '\0';
                result = 0;
                running = false;
            }
            break;

        case INPUT_QUIT:
            result = -1;
            running = false;
            break;

        default:
            // 키 코드를 확인하여 문자 입력 처리
            if (event.key_code == KEY_BACKSPACE || event.key_code == 127 || event.key_code == 8)
            {
                spectator_input_ui_handle_backspace(&ui);
            }
            else if (event.key_code >= 32 && event.key_code < 127)
            {
                spectator_input_ui_handle_char(&ui, (char)event.key_code);
            }
            break;
        }
    }

    input_handler_cleanup(&input_handler);
    delwin(input_win);
    endwin();

    return result;
}

// ==========================================
// 연결 대기 UI
// ==========================================

void connection_wait_ui_init(ConnectionWaitUI *ui, const char *ip, int port, const char *name, bool is_spectator)
{
    if (!ui)
        return;

    memset(ui, 0, sizeof(ConnectionWaitUI));
    strncpy(ui->ip_address, ip, MAX_IP_LENGTH - 1);
    ui->port = port;
    strncpy(ui->player_name, name, MAX_NAME_LENGTH - 1);
    ui->elapsed_seconds = 0;
    ui->timeout_seconds = 10;
    ui->state = CONNECT_STATE_WAITING;
    ui->is_spectator = is_spectator;
    memset(ui->error_message, 0, sizeof(ui->error_message));
}

void connection_wait_ui_render(WINDOW *win, const ConnectionWaitUI *ui)
{
    if (!win || !ui)
        return;

    wclear(win);

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    // 외곽 박스
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    box(win, 0, 0);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 타이틀
    const char *title = ui->is_spectator ? "SPECTATOR MODE" : "MULTIPLAYER";
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(win, 2, (max_x - 26) / 2, "╔═══════════════════════╗");
    mvwprintw(win, 3, (max_x - 26) / 2, "║  %-19s  ║", title);
    mvwprintw(win, 4, (max_x - 26) / 2, "╚═══════════════════════╝");
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 연결 정보 박스
    int box_y = 8;
    int box_w = 60;
    int box_h = 12;
    int box_x = (max_x - box_w) / 2;

    // 박스 테두리
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_INFO));
    mvwaddch(win, box_y, box_x, ACS_ULCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y, box_x + box_w - 1, ACS_URCORNER);

    for (int j = 1; j < box_h - 1; j++)
    {
        mvwaddch(win, box_y + j, box_x, ACS_VLINE);
        mvwaddch(win, box_y + j, box_x + box_w - 1, ACS_VLINE);
    }

    mvwaddch(win, box_y + box_h - 1, box_x, ACS_LLCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y + box_h - 1, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y + box_h - 1, box_x + box_w - 1, ACS_LRCORNER);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_INFO));

    // 연결 정보 표시
    int info_y = box_y + 2;
    wattron(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(win, info_y, box_x + 8, "Server:   %s:%d", ui->ip_address, ui->port);
    mvwprintw(win, info_y + 1, box_x + 8, "Nickname: %s", ui->player_name);
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 로딩 애니메이션
    int anim_y = box_y + 5;
    const char *loading_frames[] = {"◐", "◓", "◑", "◒"};
    int frame = ui->elapsed_seconds % 4;

    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE));
    mvwprintw(win, anim_y, (max_x - 20) / 2, "%s  Connecting...  %s",
              loading_frames[frame], loading_frames[(frame + 2) % 4]);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE));

    // 진행 바
    int bar_y = box_y + 7;
    int bar_width = 40;
    int bar_x = (max_x - bar_width) / 2;
    int filled = (ui->elapsed_seconds * bar_width) / ui->timeout_seconds;
    if (filled > bar_width)
        filled = bar_width;

    mvwprintw(win, bar_y, bar_x - 1, "[");
    for (int i = 0; i < bar_width; i++)
    {
        if (i < filled)
        {
            wattron(win, COLOR_PAIR(COLOR_PAIR_INFO) | A_BOLD);
            wprintw(win, "█");
            wattroff(win, COLOR_PAIR(COLOR_PAIR_INFO) | A_BOLD);
        }
        else
        {
            wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
            wprintw(win, "░");
            wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
        }
    }
    wprintw(win, "]");

    // 남은 시간 표시
    int remaining = ui->timeout_seconds - ui->elapsed_seconds;
    if (remaining < 0)
        remaining = 0;
    wattron(win, COLOR_PAIR(COLOR_PAIR_INFO));
    mvwprintw(win, bar_y + 1, (max_x - 12) / 2, "Timeout: %2ds", remaining);
    wattroff(win, COLOR_PAIR(COLOR_PAIR_INFO));

    // 안내 메시지
    wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
    mvwprintw(win, max_y - 3, (max_x - 22) / 2, "Press Q to cancel");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));

    wrefresh(win);
}

void connection_fail_ui_render(WINDOW *win, const ConnectionWaitUI *ui)
{
    if (!win || !ui)
        return;

    wclear(win);

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    // 외곽 박스
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    box(win, 0, 0);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 타이틀
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_BLACK_STONE));
    mvwprintw(win, 2, (max_x - 26) / 2, "╔═══════════════════════╗");
    mvwprintw(win, 3, (max_x - 26) / 2, "║   CONNECTION FAILED   ║");
    mvwprintw(win, 4, (max_x - 26) / 2, "╚═══════════════════════╝");
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_BLACK_STONE));

    // 실패 박스
    int box_y = 8;
    int box_w = 60;
    int box_h = 12;
    int box_x = (max_x - box_w) / 2;

    // 박스 테두리 (빨간색)
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_BLACK_STONE));
    mvwaddch(win, box_y, box_x, ACS_ULCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y, box_x + box_w - 1, ACS_URCORNER);

    for (int j = 1; j < box_h - 1; j++)
    {
        mvwaddch(win, box_y + j, box_x, ACS_VLINE);
        mvwaddch(win, box_y + j, box_x + box_w - 1, ACS_VLINE);
    }

    mvwaddch(win, box_y + box_h - 1, box_x, ACS_LLCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y + box_h - 1, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y + box_h - 1, box_x + box_w - 1, ACS_LRCORNER);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_BLACK_STONE));

    // 실패 아이콘
    int icon_y = box_y + 2;
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_BLACK_STONE));
    mvwprintw(win, icon_y, (max_x - 5) / 2, "\\ /");
    mvwprintw(win, icon_y + 1, (max_x - 7) / 2, "  ╳  ");
    mvwprintw(win, icon_y + 2, (max_x - 5) / 2, "/ \\");
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_BLACK_STONE));

    // 오류 메시지
    int msg_y = box_y + 6;
    wattron(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(win, msg_y, (max_x - 28) / 2, "Could not connect to server");
    mvwprintw(win, msg_y + 1, (max_x - strlen(ui->ip_address) - 18) / 2, "IP: %s    PORT: %d", ui->ip_address, ui->port);
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 상세 오류 메시지
    if (strlen(ui->error_message) > 0)
    {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
        mvwprintw(win, msg_y + 3, (max_x - strlen(ui->error_message)) / 2, "%s", ui->error_message);
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));
    }

    // 안내 메시지
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE));
    mvwprintw(win, max_y - 6, (max_x - 18) / 2, "Press Q to go back");
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE));

    wrefresh(win);
}

// 멀티플레이어 연결 대기 (10초 타임아웃)
int multiplay_wait_connection(const char *ip, int port, const char *name)
{
    if (!ip || !name)
        return -1;

    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    if (has_colors())
    {
        start_color();
        ThemeType saved_theme = theme_load_from_config();
        theme_init(saved_theme);
    }

    wresize(stdscr, UI_MIN_HEIGHT, UI_MIN_WIDTH);
    WINDOW *wait_win = newwin(UI_MIN_HEIGHT, UI_MIN_WIDTH, 0, 0);
    keypad(wait_win, TRUE);
    nodelay(wait_win, TRUE); // 논블로킹 입력

    InputHandler input_handler;
    input_handler_init(&input_handler);

    ConnectionWaitUI ui;
    connection_wait_ui_init(&ui, ip, port, name, false);

    // 네트워크 초기화
    NetworkManager network;
    if (!network_init_client(&network))
    {
        strncpy(ui.error_message, "Failed to initialize network", sizeof(ui.error_message) - 1);
        ui.error_message[sizeof(ui.error_message) - 1] = '\0';
        ui.state = CONNECT_STATE_FAILED;
    }

    time_t start_time = time(NULL);
    bool running = true;
    int result = -1;
    bool connection_attempted = false;
    bool socket_created = false;

    // 소켓 생성
    if (ui.state != CONNECT_STATE_FAILED)
    {
        network.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (network.socket_fd < 0)
        {
            strncpy(ui.error_message, "Failed to create socket", sizeof(ui.error_message) - 1);
            ui.error_message[sizeof(ui.error_message) - 1] = '\0';
            ui.state = CONNECT_STATE_FAILED;
        }
        else
        {
            socket_created = true;
            // 논블로킹 모드로 설정
            network_set_nonblocking(network.socket_fd, true);

            // 서버 주소 설정 및 연결 시작
            struct sockaddr_in server_addr;
            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);

            if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
            {
                strncpy(ui.error_message, "Invalid IP address", sizeof(ui.error_message) - 1);
                ui.error_message[sizeof(ui.error_message) - 1] = '\0';
                ui.state = CONNECT_STATE_FAILED;
            }
            else
            {
                // 논블로킹 연결 시작
                int conn_result = connect(network.socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
                if (conn_result < 0 && errno != EINPROGRESS)
                {
                    strncpy(ui.error_message, "Connection refused", sizeof(ui.error_message) - 1);
                    ui.error_message[sizeof(ui.error_message) - 1] = '\0';
                    ui.state = CONNECT_STATE_FAILED;
                }
                else
                {
                    connection_attempted = true;
                }
            }
        }
    }

    bool fail_rendered = false; // 실패 화면 렌더링 여부

    while (running)
    {
        // 경과 시간 업데이트
        ui.elapsed_seconds = (int)difftime(time(NULL), start_time);

        // 타임아웃 체크
        if (ui.elapsed_seconds >= ui.timeout_seconds && ui.state == CONNECT_STATE_WAITING)
        {
            strncpy(ui.error_message, "Connection timed out", sizeof(ui.error_message) - 1);
            ui.error_message[sizeof(ui.error_message) - 1] = '\0';
            ui.state = CONNECT_STATE_FAILED;
        }

        // UI 렌더링
        if (ui.state == CONNECT_STATE_WAITING)
        {
            connection_wait_ui_render(wait_win, &ui);
        }
        else if (ui.state == CONNECT_STATE_FAILED && !fail_rendered)
        {
            // 실패 화면은 한 번만 렌더링
            connection_fail_ui_render(wait_win, &ui);
            fail_rendered = true;
        }

        // 연결 상태 확인 (논블로킹)
        if (ui.state == CONNECT_STATE_WAITING && connection_attempted)
        {
            fd_set write_fds;
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 100000; // 100ms

            FD_ZERO(&write_fds);
            FD_SET(network.socket_fd, &write_fds);

            int sel_result = select(network.socket_fd + 1, NULL, &write_fds, NULL, &tv);
            if (sel_result > 0)
            {
                // 연결 결과 확인
                int so_error;
                socklen_t len = sizeof(so_error);
                getsockopt(network.socket_fd, SOL_SOCKET, SO_ERROR, &so_error, &len);

                if (so_error == 0)
                {
                    // 연결 성공!
                    ui.state = CONNECT_STATE_SUCCESS;
                    result = 0;
                    running = false;

                    // 블로킹 모드로 복원
                    network_set_nonblocking(network.socket_fd, false);
                }
                else
                {
                    strncpy(ui.error_message, "Connection refused", sizeof(ui.error_message) - 1);
                    ui.error_message[sizeof(ui.error_message) - 1] = '\0';
                    ui.state = CONNECT_STATE_FAILED;
                }
            }
        }

        // 입력 처리
        int ch = wgetch(wait_win);
        if (ch != ERR)
        {
            if (ch == 'q' || ch == 'Q' || ch == 27) // Q 또는 ESC
            {
                if (ui.state == CONNECT_STATE_FAILED)
                {
                    // 실패 화면에서 나가기
                    result = -1;
                    running = false;
                }
                else
                {
                    // 대기 중 취소
                    ui.state = CONNECT_STATE_CANCELLED;
                    result = -1;
                    running = false;
                }
            }
        }

        usleep(50000); // 50ms 대기
    }

    // 정리
    if (socket_created && result != 0)
    {
        close(network.socket_fd);
    }

    input_handler_cleanup(&input_handler);
    delwin(wait_win);
    endwin();

    return result;
}

// 관전자 연결 대기 (10초 타임아웃)
int spectator_wait_connection(const char *ip, int port, const char *name)
{
    if (!ip || !name)
        return -1;

    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    if (has_colors())
    {
        start_color();
        ThemeType saved_theme = theme_load_from_config();
        theme_init(saved_theme);
    }

    wresize(stdscr, UI_MIN_HEIGHT, UI_MIN_WIDTH);
    WINDOW *wait_win = newwin(UI_MIN_HEIGHT, UI_MIN_WIDTH, 0, 0);
    keypad(wait_win, TRUE);
    nodelay(wait_win, TRUE); // 논블로킹 입력

    InputHandler input_handler;
    input_handler_init(&input_handler);

    ConnectionWaitUI ui;
    connection_wait_ui_init(&ui, ip, port, name, true);

    // 네트워크 초기화
    NetworkManager network;
    if (!network_init_spectator(&network))
    {
        strncpy(ui.error_message, "Failed to initialize network", sizeof(ui.error_message) - 1);
        ui.error_message[sizeof(ui.error_message) - 1] = '\0';
        ui.state = CONNECT_STATE_FAILED;
    }

    time_t start_time = time(NULL);
    bool running = true;
    int result = -1;
    bool connection_attempted = false;
    bool socket_created = false;

    // 소켓 생성
    if (ui.state != CONNECT_STATE_FAILED)
    {
        network.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (network.socket_fd < 0)
        {
            strncpy(ui.error_message, "Failed to create socket", sizeof(ui.error_message) - 1);
            ui.error_message[sizeof(ui.error_message) - 1] = '\0';
            ui.state = CONNECT_STATE_FAILED;
        }
        else
        {
            socket_created = true;
            // 논블로킹 모드로 설정
            network_set_nonblocking(network.socket_fd, true);

            // 서버 주소 설정 및 연결 시작
            struct sockaddr_in server_addr;
            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);

            if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
            {
                strncpy(ui.error_message, "Invalid IP address", sizeof(ui.error_message) - 1);
                ui.error_message[sizeof(ui.error_message) - 1] = '\0';
                ui.state = CONNECT_STATE_FAILED;
            }
            else
            {
                // 논블로킹 연결 시작
                int conn_result = connect(network.socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
                if (conn_result < 0 && errno != EINPROGRESS)
                {
                    strncpy(ui.error_message, "Connection refused", sizeof(ui.error_message) - 1);
                    ui.error_message[sizeof(ui.error_message) - 1] = '\0';
                    ui.state = CONNECT_STATE_FAILED;
                }
                else
                {
                    connection_attempted = true;
                }
            }
        }
    }

    bool fail_rendered = false; // 실패 화면 렌더링 여부

    while (running)
    {
        // 경과 시간 업데이트
        ui.elapsed_seconds = (int)difftime(time(NULL), start_time);

        // 타임아웃 체크
        if (ui.elapsed_seconds >= ui.timeout_seconds && ui.state == CONNECT_STATE_WAITING)
        {
            strncpy(ui.error_message, "Connection timed out", sizeof(ui.error_message) - 1);
            ui.error_message[sizeof(ui.error_message) - 1] = '\0';
            ui.state = CONNECT_STATE_FAILED;
        }

        // UI 렌더링
        if (ui.state == CONNECT_STATE_WAITING)
        {
            connection_wait_ui_render(wait_win, &ui);
        }
        else if (ui.state == CONNECT_STATE_FAILED && !fail_rendered)
        {
            // 실패 화면은 한 번만 렌더링
            connection_fail_ui_render(wait_win, &ui);
            fail_rendered = true;
        }

        // 연결 상태 확인 (논블로킹)
        if (ui.state == CONNECT_STATE_WAITING && connection_attempted)
        {
            fd_set write_fds;
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 100000; // 100ms

            FD_ZERO(&write_fds);
            FD_SET(network.socket_fd, &write_fds);

            int sel_result = select(network.socket_fd + 1, NULL, &write_fds, NULL, &tv);
            if (sel_result > 0)
            {
                // 연결 결과 확인
                int so_error;
                socklen_t len = sizeof(so_error);
                getsockopt(network.socket_fd, SOL_SOCKET, SO_ERROR, &so_error, &len);

                if (so_error == 0)
                {
                    // 연결 성공!
                    ui.state = CONNECT_STATE_SUCCESS;
                    result = 0;
                    running = false;

                    // 블로킹 모드로 복원
                    network_set_nonblocking(network.socket_fd, false);
                }
                else
                {
                    strncpy(ui.error_message, "Connection refused", sizeof(ui.error_message) - 1);
                    ui.error_message[sizeof(ui.error_message) - 1] = '\0';
                    ui.state = CONNECT_STATE_FAILED;
                }
            }
        }

        // 입력 처리
        int ch = wgetch(wait_win);
        if (ch != ERR)
        {
            if (ch == 'q' || ch == 'Q' || ch == 27) // Q 또는 ESC
            {
                if (ui.state == CONNECT_STATE_FAILED)
                {
                    // 실패 화면에서 나가기
                    result = -1;
                    running = false;
                }
                else
                {
                    // 대기 중 취소
                    ui.state = CONNECT_STATE_CANCELLED;
                    result = -1;
                    running = false;
                }
            }
        }

        usleep(50000); // 50ms 대기
    }

    // 정리 - 연결 성공/실패 상관없이 소켓 닫기
    // (spectator_run에서 새로 연결하므로 여기서는 연결 가능 여부만 확인)
    if (socket_created)
    {
        close(network.socket_fd);
    }

    input_handler_cleanup(&input_handler);
    delwin(wait_win);
    endwin();

    return result;
}

// ==========================================
// HOST 대기 UI
// ==========================================

void host_wait_ui_init(HostWaitUI *ui, const char *local_ip, int port, const char *host_name)
{
    if (!ui)
        return;

    memset(ui, 0, sizeof(HostWaitUI));
    strncpy(ui->local_ip, local_ip, MAX_IP_LENGTH - 1);
    ui->port = port;
    strncpy(ui->host_name, host_name, MAX_NAME_LENGTH - 1);
    ui->elapsed_seconds = 0;
    ui->state = CONNECT_STATE_WAITING;
    memset(ui->connected_client, 0, sizeof(ui->connected_client));
}

void host_wait_ui_render(WINDOW *win, const HostWaitUI *ui)
{
    if (!win || !ui)
        return;

    wclear(win);

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    // 외곽 박스
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    box(win, 0, 0);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 타이틀
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(win, 2, (max_x - 26) / 2, "╔═══════════════════════╗");
    mvwprintw(win, 3, (max_x - 26) / 2, "║    HOST GAME (LAN)    ║");
    mvwprintw(win, 4, (max_x - 26) / 2, "╚═══════════════════════╝");
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 서버 정보 박스
    int box_y = 7;
    int box_w = 60;
    int box_h = 14;
    int box_x = (max_x - box_w) / 2;

    // 박스 테두리
    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_INFO));
    mvwaddch(win, box_y, box_x, ACS_ULCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y, box_x + box_w - 1, ACS_URCORNER);

    for (int j = 1; j < box_h - 1; j++)
    {
        mvwaddch(win, box_y + j, box_x, ACS_VLINE);
        mvwaddch(win, box_y + j, box_x + box_w - 1, ACS_VLINE);
    }

    mvwaddch(win, box_y + box_h - 1, box_x, ACS_LLCORNER);
    for (int i = 1; i < box_w - 1; i++)
        mvwaddch(win, box_y + box_h - 1, box_x + i, ACS_HLINE);
    mvwaddch(win, box_y + box_h - 1, box_x + box_w - 1, ACS_LRCORNER);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_INFO));

    // 서버 정보 표시
    int info_y = box_y + 2;
    wattron(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(win, info_y, box_x + 6, "Server IP:   %s", ui->local_ip);
    mvwprintw(win, info_y + 1, box_x + 6, "Port:        %d", ui->port);
    mvwprintw(win, info_y + 2, box_x + 6, "Host Name:   %s", ui->host_name);
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // 구분선
    wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
    mvwprintw(win, info_y + 4, box_x + 4, "────────────────────────────────────────────────────");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));

    // 안내 메시지 (정적)
    wattron(win, COLOR_PAIR(COLOR_PAIR_DIM));
    mvwprintw(win, max_y - 5, (max_x - 48) / 2, "Share your IP address with your opponent to join");
    mvwprintw(win, max_y - 3, (max_x - 22) / 2, "Press Q to cancel");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DIM));

    // 동적 부분 렌더링
    host_wait_ui_render_dynamic(win, ui);

    wrefresh(win);
}

// 동적 부분만 업데이트 (애니메이션, 경과 시간)
void host_wait_ui_render_dynamic(WINDOW *win, const HostWaitUI *ui)
{
    if (!win || !ui)
        return;

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    (void)max_y; // unused

    int box_y = 7;
    int anim_y = box_y + 8;

    // 원형 로딩 애니메이션 (0.25초마다)
    const char *loading_frames[] = {"◜ ", " ◝", " ◞", "◟ "};
    // 점 애니메이션 (0.5초마다)
    const char *dot_frames[] = {"   ", ".  ", ".. ", "..."};
    int spin_frame = ui->anim_frame % 4;
    int dot_frame = ui->dot_frame % 4;

    // 이전 텍스트 지우기 (공백으로 덮기)
    mvwprintw(win, anim_y, (max_x - 34) / 2, "                                  ");

    wattron(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE));
    mvwprintw(win, anim_y, (max_x - 31) / 2, "%s Waiting for opponent %s %s",
              loading_frames[spin_frame], dot_frames[dot_frame], loading_frames[(spin_frame + 2) % 4]);
    wattroff(win, A_BOLD | COLOR_PAIR(COLOR_PAIR_WHITE_STONE));

    // 경과 시간 표시
    int minutes = ui->elapsed_seconds / 60;
    int seconds = ui->elapsed_seconds % 60;
    wattron(win, COLOR_PAIR(COLOR_PAIR_INFO));
    mvwprintw(win, anim_y + 2, (max_x - 16) / 2, "Elapsed: %02d:%02d", minutes, seconds);
    wattroff(win, COLOR_PAIR(COLOR_PAIR_INFO));

    wrefresh(win);
}

// HOST 대기 화면 실행 (클라이언트 연결 대기)
int multiplay_host_wait_for_client(int port, const char *host_name, GameRule rule)
{
    if (!host_name)
        return -1;

    if (port == 0)
        port = DEFAULT_PORT;

    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    if (has_colors())
    {
        start_color();
        ThemeType saved_theme = theme_load_from_config();
        theme_init(saved_theme);
    }

    wresize(stdscr, UI_MIN_HEIGHT, UI_MIN_WIDTH);
    WINDOW *wait_win = newwin(UI_MIN_HEIGHT, UI_MIN_WIDTH, 0, 0);
    keypad(wait_win, TRUE);
    nodelay(wait_win, TRUE); // 논블로킹 입력

    InputHandler input_handler;
    input_handler_init(&input_handler);

    // 로컬 IP 가져오기
    char local_ip[MAX_IP_LENGTH];
    network_get_local_ip(local_ip, sizeof(local_ip));

    HostWaitUI ui;
    host_wait_ui_init(&ui, local_ip, port, host_name);

    // 네트워크 초기화
    NetworkManager network;
    if (!network_init_server(&network, port))
    {
        ui.state = CONNECT_STATE_FAILED;
        input_handler_cleanup(&input_handler);
        delwin(wait_win);
        endwin();
        return -1;
    }

    if (!network_server_start_listen(&network))
    {
        ui.state = CONNECT_STATE_FAILED;
        network_cleanup(&network);
        input_handler_cleanup(&input_handler);
        delwin(wait_win);
        endwin();
        return -1;
    }

    // 논블로킹 모드 설정
    network_set_nonblocking(network.socket_fd, true);

    time_t start_time = time(NULL);
    struct timespec last_anim_time, last_dot_time;
    clock_gettime(CLOCK_MONOTONIC, &last_anim_time);
    last_dot_time = last_anim_time;
    bool running = true;
    int result = -1;
    bool static_rendered = false; // 정적 부분 렌더링 플래그
    ui.anim_frame = 0;
    ui.dot_frame = 0;

    while (running)
    {
        time_t current_time = time(NULL);
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        // 경과 시간 업데이트 (1초 단위)
        ui.elapsed_seconds = (int)difftime(current_time, start_time);

        // 원형 애니메이션 프레임 업데이트 (0.25초마다)
        long elapsed_anim_ms = (now.tv_sec - last_anim_time.tv_sec) * 1000 +
                               (now.tv_nsec - last_anim_time.tv_nsec) / 1000000;
        if (elapsed_anim_ms >= 250)
        {
            ui.anim_frame = (ui.anim_frame + 1) % 4;
            last_anim_time = now;
        }

        // 점 애니메이션 프레임 업데이트 (0.5초마다)
        long elapsed_dot_ms = (now.tv_sec - last_dot_time.tv_sec) * 1000 +
                              (now.tv_nsec - last_dot_time.tv_nsec) / 1000000;
        if (elapsed_dot_ms >= 500)
        {
            ui.dot_frame = (ui.dot_frame + 1) % 4;
            last_dot_time = now;
        }

        // 정적 부분은 처음 한 번만 렌더링
        if (!static_rendered)
        {
            host_wait_ui_render(wait_win, &ui);
            static_rendered = true;
        }
        // 이후에는 동적 부분만 업데이트 (매 루프마다 - 애니메이션 부드럽게)
        else
        {
            host_wait_ui_render_dynamic(wait_win, &ui);
        }

        // 클라이언트 연결 확인
        if (network_server_accept_client(&network))
        {
            // 연결 성공!
            ui.state = CONNECT_STATE_SUCCESS;
            result = 0;
            running = false;
        }

        // 입력 처리
        int ch = wgetch(wait_win);
        if (ch != ERR)
        {
            if (ch == 'q' || ch == 'Q' || ch == 27) // Q 또는 ESC
            {
                ui.state = CONNECT_STATE_CANCELLED;
                result = -1;
                running = false;
            }
        }

        usleep(50000); // 50ms 대기
    }

    // ncurses 종료
    input_handler_cleanup(&input_handler);
    delwin(wait_win);
    endwin();

    // 정리
    if (result != 0)
    {
        network_cleanup(&network);
        return result;
    }

    // 연결 성공 시 정리 후 반환 (main.c에서 multiplayer_run_host 호출)
    network_cleanup(&network);
    return 0;
}
