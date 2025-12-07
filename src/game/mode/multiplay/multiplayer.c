#include "multiplayer.h"
#include "mp_types.h"
#include "mp_game.h"
#include "mp_input.h"
#include "mp_network.h"
#include "mp_turn.h"
#include "../../../ui/menu/menu_ui.h"
#include "../../../utils/terminal_check.h"
#include "../../../ui/game/border/ingame_border.h"
#include "../../../ui/core/ui_manager.h"
#include "../../../ui/core/theme.h"
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>

// ============================================================================
// 호스트 모드 실행
// ============================================================================

int multiplayer_run_host(int port, GameRule rule, const char *player_name)
{
    if (port == 0)
        port = DEFAULT_PORT;

    MultiplayerGame game;
    memset(&game, 0, sizeof(game));

    // 네트워크 초기화
    if (!network_init_server(&game.network, port))
    {
        printf("Failed to initialize server\n");
        return -1;
    }

    if (!network_server_start_listen(&game.network))
    {
        printf("Failed to start listening\n");
        network_cleanup(&game.network);
        return -1;
    }

    // 대기 화면을 위한 ncurses 초기화
    initscr();
    start_color();
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE); // non-blocking 입력

    // 테마 초기화
    ThemeType saved_theme = theme_load_from_config();
    theme_init(saved_theme);

    network_set_nonblocking(game.network.socket_fd, true);

    // 애니메이션 프레임
    const char *spinner[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
    const int spinner_frames = 10;
    int frame = 0;
    int dots = 0;
    time_t start_time = time(NULL);
    bool connected = false;
    bool cancelled = false;

    while (!connected && !cancelled)
    {
        // 클라이언트 연결 확인
        if (network_server_accept_client(&game.network))
        {
            connected = true;
            break;
        }

        // 키 입력 확인 (취소)
        int ch = getch();
        if (ch == 'q' || ch == 'Q' || ch == 27) // ESC
        {
            cancelled = true;
            break;
        }

        // 화면 렌더링
        clear();

        menu_ui_draw_logo(stdscr);

        // 전체 터미널을 채우는 border box 그리기 (100x31)
        int box_width = 100;
        int box_height = 31;
        int box_start_x = 0;
        int box_start_y = 0;

        // 테두리 색상
        attron(COLOR_PAIR(COLOR_PAIR_DEFAULT) | A_BOLD);

        // 상단 테두리
        mvprintw(box_start_y, box_start_x, "┏");
        for (int i = 1; i < box_width - 1; i++)
            mvprintw(box_start_y, box_start_x + i, "━");
        mvprintw(box_start_y, box_start_x + box_width - 1, "┓");

        // 측면 테두리
        for (int i = 1; i < box_height - 1; i++)
        {
            mvprintw(box_start_y + i, box_start_x, "┃");
            mvprintw(box_start_y + i, box_start_x + box_width - 1, "┃");
        }

        // 하단 테두리
        mvprintw(box_start_y + box_height - 1, box_start_x, "┗");
        for (int i = 1; i < box_width - 1; i++)
            mvprintw(box_start_y + box_height - 1, box_start_x + i, "━");
        mvprintw(box_start_y + box_height - 1, box_start_x + box_width - 1, "┛");

        attroff(COLOR_PAIR(COLOR_PAIR_DEFAULT) | A_BOLD);

        // 서버 정보 박스 (중앙)
        int info_box_width = 60;
        int info_box_height = 12;
        int info_box_start_x = box_start_x + (box_width - info_box_width) / 2;
        int info_box_start_y = box_start_y + 15;

        // 정보 박스 테두리
        mvprintw(info_box_start_y, info_box_start_x, "┌");
        for (int i = 1; i < info_box_width - 1; i++)
            mvprintw(info_box_start_y, info_box_start_x + i, "─");
        mvprintw(info_box_start_y, info_box_start_x + info_box_width - 1, "┐");

        for (int i = 1; i < info_box_height - 1; i++)
        {
            mvprintw(info_box_start_y + i, info_box_start_x, "│");
            mvprintw(info_box_start_y + i, info_box_start_x + info_box_width - 1, "│");
        }

        mvprintw(info_box_start_y + info_box_height - 1, info_box_start_x, "└");
        for (int i = 1; i < info_box_width - 1; i++)
            mvprintw(info_box_start_y + info_box_height - 1, info_box_start_x + i, "─");
        mvprintw(info_box_start_y + info_box_height - 1, info_box_start_x + info_box_width - 1, "┘");

        // 서버 정보 내용
        attron(COLOR_PAIR(COLOR_PAIR_DEFAULT) | A_BOLD);
        mvprintw(info_box_start_y + 1, info_box_start_x + 2, "Host Mode: Server Information");
        attroff(COLOR_PAIR(COLOR_PAIR_DEFAULT) | A_BOLD);

        attron(COLOR_PAIR(COLOR_PAIR_DEFAULT));
        mvprintw(info_box_start_y + 3, info_box_start_x + 2, "IP Address: %s", game.network.local_ip);
        mvprintw(info_box_start_y + 4, info_box_start_x + 2, "Port: %d", port);
        mvprintw(info_box_start_y + 5, info_box_start_x + 2, "Host: %s", player_name);
        mvprintw(info_box_start_y + 6, info_box_start_x + 2, "Rule: %s", rule == RULE_STANDARD ? "Standard" : "Renju");
        attroff(COLOR_PAIR(COLOR_PAIR_DEFAULT));

        // 스피너와 대기 메시지
        attron(COLOR_PAIR(COLOR_PAIR_SYSTEM) | A_BOLD);
        mvprintw(info_box_start_y + 8, info_box_start_x + (info_box_width - 2) / 2, "%s", spinner[frame]);
        mvprintw(info_box_start_y + 9, info_box_start_x + (info_box_width - 20) / 2, "Waiting for player...");
        attroff(COLOR_PAIR(COLOR_PAIR_SYSTEM) | A_BOLD);

        // 안내 메시지 (하단)
        attron(COLOR_PAIR(COLOR_PAIR_SYSTEM) | A_DIM);
        mvprintw(box_start_y + box_height - 3, box_start_x + (box_width - 24) / 2, "Press Q or ESC to cancel");
        attroff(COLOR_PAIR(COLOR_PAIR_SYSTEM) | A_DIM);

        refresh();

        // 애니메이션 업데이트
        frame = (frame + 1) % spinner_frames;
        dots++;

        usleep(100000); // 100ms
    }

    endwin();

    if (cancelled)
    {
        network_cleanup(&game.network);
        return -1;
    }

    // 연결 성공 - 잠시 메시지 표시
    initscr();
    start_color();
    cbreak();
    noecho();
    curs_set(0);
    theme_init(saved_theme);

    clear();
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(LINES / 2 - 1, (COLS - 18) / 2, "PLAYER CONNECTED!");
    attroff(COLOR_PAIR(3) | A_BOLD);

    attron(COLOR_PAIR(2));
    mvprintw(LINES / 2 + 1, (COLS - 30) / 2, "Client: %s:%d", game.network.remote_ip, game.network.remote_port);
    attroff(COLOR_PAIR(2));

    attron(A_DIM);
    mvprintw(LINES / 2 + 3, (COLS - 18) / 2, "Starting game...");
    attroff(A_DIM);

    refresh();
    usleep(1500000); // 1.5초 대기
    endwin();

    // 플레이어 정보 설정 (TUI에서 받은 닉네임 사용)
    strncpy(game.me.name, player_name, MAX_PLAYER_NAME - 1);
    game.me.name[MAX_PLAYER_NAME - 1] = '\0';
    if (strlen(game.me.name) == 0)
    {
        strcpy(game.me.name, "Host");
    }
    game.me.color = BLACK;

    strcpy(game.opponent.name, "Client");
    game.opponent.color = WHITE;

    // 연결 승인 메시지 전송
    Message msg;
    protocol_init_message(&msg, MSG_CONNECT_ACK, game.network.sequence_number++);
    msg.payload.connect_ack.your_color = WHITE;
    msg.payload.connect_ack.game_rule = rule;
    strncpy(msg.payload.connect_ack.opponent_name, game.me.name, MAX_PLAYER_NAME);
    network_send_message(&game.network, &msg);

    // 상대방 이름 수신 대기
    int attempts = 0;
    while (attempts < 50)
    {
        Message recv_msg;
        int result = network_receive_message(&game.network, &recv_msg);
        if (result > 0 && recv_msg.header.type == MSG_PLAYER_INFO)
        {
            strncpy(game.opponent.name, recv_msg.payload.player_info.name, MAX_PLAYER_NAME);
            break;
        }
        usleep(100000);
        attempts++;
    }

    // UI 및 게임 컴포넌트 초기화
    UIManager ui_mgr;
    if (!mp_init_game_ui(&ui_mgr, &game, rule))
    {
        printf("Failed to initialize UI\n");
        network_cleanup(&game.network);
        return -1;
    }

    char start_msg[128];
    snprintf(start_msg, sizeof(start_msg), "Game started! You: BLACK");
    log_add_msg(&game.log_ui, start_msg);
    chat_add_msg(&game.chat_ui, start_msg, CHAT_MSG_SYSTEM);

    InputHandler input_handler;
    input_handler_init(&input_handler);

    // 게임 시작 메시지 전송
    protocol_init_message(&msg, MSG_GAME_START, game.network.sequence_number++);
    msg.payload.game_start.your_turn = 0;
    network_send_message(&game.network, &msg);

    bool game_running = true;

    while (game_running)
    {
        // ============================================
        // 터미널 크기 체크 (멀티플레이: 경고만, 게임 계속 진행)
        // ============================================
        TerminalSizeStatus term_status = check_terminal_size_ingame();

        if (term_status == TERMINAL_SIZE_TOO_SMALL && !game.terminal_warning_shown)
        {
            // 터미널이 작아짐 - 로그에 경고만 표시 (게임은 계속 진행)
            log_add_msg(&game.log_ui, "[WARNING] Terminal too small!");
            log_add_msg(&game.log_ui, "Please resize to continue.");
            game.terminal_warning_shown = true;
        }
        else if (term_status == TERMINAL_SIZE_RESTORED && game.terminal_warning_shown)
        {
            // 터미널 크기 복원됨
            log_add_msg(&game.log_ui, "Terminal size restored.");
            game.terminal_warning_shown = false;

            // 전체 화면 다시 그리기
            clear();
            refresh();
            ingame_border_draw();
            refresh();
            game.first_render = true;
        }

        mp_handle_spectator_connections(&game);
        mp_send_ping_if_needed(&game);
        mp_handle_network_messages(&ui_mgr, &game);
        mp_render_game(&ui_mgr, &game);

        mp_check_game_end(&game, true); // is_host = true

        // 턴 처리
        Stone current_player = turn_manager_get_current_player(&game.turn_mgr);
        if (!game.game_over && current_player == game.me.color)
        {
            if (mp_handle_my_turn(&ui_mgr, &game, &input_handler))
            {
                turn_manager_next_turn(&game.turn_mgr);
                // 턴 변경 후 System Log에 메시지
                Stone next_player = turn_manager_get_current_player(&game.turn_mgr);
                char turn_msg[32];
                const char *player_name = (next_player == game.me.color) ? game.me.name : game.opponent.name;
                snprintf(turn_msg, sizeof(turn_msg), "%s's turn", player_name);
                log_add_msg(&game.log_ui, turn_msg);
            }
        }
        else if (!game.game_over)
        {
            mp_handle_opponent_turn(&ui_mgr, &game, &input_handler);
        }

        // 게임 종료 후 처리
        if (game.game_over)
        {
            if (mp_handle_game_over_input(&ui_mgr, &game, &input_handler))
            {
                game_running = false;
            }
        }
    }

    mp_cleanup_game(&ui_mgr, &game, &input_handler);
    return 0;
}

// ============================================================================
// 클라이언트 모드 실행
// ============================================================================

int multiplayer_run_client(const char *server_ip, int port, GameRule rule, const char *player_name)
{
    MultiplayerGame game;
    memset(&game, 0, sizeof(game));
    GameRule received_rule = rule;

    if (!network_init_client(&game.network))
    {
        printf("Failed to initialize client\n");
        return -1;
    }

    // CLI 모드에서만 printf 사용 (TUI에서 호출 시 주석 처리)
    // printf("=== GOMOKU MULTIPLAYER - CLIENT ===\n");
    // printf("Connecting to %s:%d...\n", server_ip, port);

    if (!network_client_connect(&game.network, server_ip, port))
    {
        // 연결 실패 화면 표시
        initscr();
        start_color();
        cbreak();
        noecho();
        curs_set(0);

        // 테마 초기화
        ThemeType saved_theme = theme_load_from_config();
        theme_init(saved_theme);

        clear();

        // 타이틀
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(LINES / 2 - 3, (COLS - 20) / 2, "CONNECTION FAILED");
        attroff(COLOR_PAIR(1) | A_BOLD);

        // 에러 메시지
        attron(COLOR_PAIR(2));
        mvprintw(LINES / 2 - 1, (COLS - 30) / 2, "Failed to connect to server");
        mvprintw(LINES / 2, (COLS - 30) / 2, "Server: %s:%d", server_ip, port);
        attroff(COLOR_PAIR(2));

        // 안내 메시지
        attron(A_DIM);
        mvprintw(LINES / 2 + 3, (COLS - 25) / 2, "Press any key to return");
        attroff(A_DIM);

        refresh();
        timeout(-1); // blocking mode
        getch();

        endwin();
        network_cleanup(&game.network);
        return -1;
    }

    // printf("Connected to server!\n");

    // 플레이어 정보 설정 (TUI에서 받은 닉네임 사용)
    strncpy(game.me.name, player_name, MAX_PLAYER_NAME - 1);
    game.me.name[MAX_PLAYER_NAME - 1] = '\0';
    if (strlen(game.me.name) == 0)
    {
        strcpy(game.me.name, "Client");
    }

    // 연결 승인 대기
    Message msg;
    int attempts = 0;
    bool connected = false;
    while (attempts < 50)
    {
        int result = network_receive_message(&game.network, &msg);
        if (result > 0 && msg.header.type == MSG_CONNECT_ACK)
        {
            game.me.color = msg.payload.connect_ack.your_color;
            received_rule = msg.payload.connect_ack.game_rule;
            strncpy(game.opponent.name, msg.payload.connect_ack.opponent_name, MAX_PLAYER_NAME);
            game.opponent.color = (game.me.color == BLACK) ? WHITE : BLACK;
            connected = true;
            break;
        }
        usleep(100000);
        attempts++;
    }

    if (!connected)
    {
        // 연결 타임아웃 화면 표시
        initscr();
        start_color();
        cbreak();
        noecho();
        curs_set(0);

        // 테마 초기화
        ThemeType saved_theme = theme_load_from_config();
        theme_init(saved_theme);

        clear();

        // 타이틀
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(LINES / 2 - 3, (COLS - 20) / 2, "CONNECTION TIMEOUT");
        attroff(COLOR_PAIR(1) | A_BOLD);

        // 에러 메시지
        attron(COLOR_PAIR(2));
        mvprintw(LINES / 2 - 1, (COLS - 40) / 2, "Server did not respond in time");
        mvprintw(LINES / 2, (COLS - 30) / 2, "Server: %s:%d", server_ip, port);
        attroff(COLOR_PAIR(2));

        // 안내 메시지
        attron(A_DIM);
        mvprintw(LINES / 2 + 3, (COLS - 25) / 2, "Press any key to return");
        attroff(A_DIM);

        refresh();
        timeout(-1); // blocking mode
        getch();

        endwin();
        network_cleanup(&game.network);
        return -1;
    }

    // 플레이어 정보 전송
    protocol_init_message(&msg, MSG_PLAYER_INFO, game.network.sequence_number++);
    strncpy(msg.payload.player_info.name, game.me.name, MAX_PLAYER_NAME);
    msg.payload.player_info.color = game.me.color;
    network_send_message(&game.network, &msg);

    printf("Assigned color: %s\n", game.me.color == BLACK ? "BLACK" : "WHITE");
    printf("Opponent: %s\n", game.opponent.name);
    printf("Starting game...\n");
    sleep(1);

    // 게임 시작 대기
    attempts = 0;
    while (attempts < 50)
    {
        int result = network_receive_message(&game.network, &msg);
        if (result > 0 && msg.header.type == MSG_GAME_START)
        {
            break;
        }
        usleep(100000);
        attempts++;
    }

    // UI 및 게임 컴포넌트 초기화
    UIManager ui_mgr;
    if (!mp_init_game_ui(&ui_mgr, &game, received_rule))
    {
        network_cleanup(&game.network);
        return -1;
    }

    char start_msg[128];
    snprintf(start_msg, sizeof(start_msg), "Game started! You: %s",
             game.me.color == BLACK ? "BLACK" : "WHITE");
    log_add_msg(&game.log_ui, start_msg);
    chat_add_msg(&game.chat_ui, start_msg, CHAT_MSG_SYSTEM);

    InputHandler input_handler;
    input_handler_init(&input_handler);

    bool game_running = true;

    while (game_running)
    {
        // ============================================
        // 터미널 크기 체크 (멀티플레이: 경고만, 게임 계속 진행)
        // ============================================
        TerminalSizeStatus term_status = check_terminal_size_ingame();

        if (term_status == TERMINAL_SIZE_TOO_SMALL && !game.terminal_warning_shown)
        {
            // 터미널이 작아짐 - 로그에 경고만 표시 (게임은 계속 진행)
            log_add_msg(&game.log_ui, "[WARNING] Terminal too small!");
            log_add_msg(&game.log_ui, "Please resize to continue.");
            game.terminal_warning_shown = true;
        }
        else if (term_status == TERMINAL_SIZE_RESTORED && game.terminal_warning_shown)
        {
            // 터미널 크기 복원됨
            log_add_msg(&game.log_ui, "Terminal size restored.");
            game.terminal_warning_shown = false;

            // 전체 화면 다시 그리기
            clear();
            refresh();
            ingame_border_draw();
            refresh();
            game.first_render = true;
        }

        mp_send_ping_if_needed(&game);
        mp_handle_network_messages(&ui_mgr, &game);
        mp_render_game(&ui_mgr, &game);

        mp_check_game_end(&game, false); // is_host = false

        Stone current_player = turn_manager_get_current_player(&game.turn_mgr);
        if (!game.game_over && current_player == game.me.color)
        {
            if (mp_handle_my_turn(&ui_mgr, &game, &input_handler))
            {
                turn_manager_next_turn(&game.turn_mgr);
                // 턴 변경 후 System Log에 메시지
                Stone next_player = turn_manager_get_current_player(&game.turn_mgr);
                char turn_msg[32];
                const char *player_name = (next_player == game.me.color) ? game.me.name : game.opponent.name;
                snprintf(turn_msg, sizeof(turn_msg), "%s's turn", player_name);
                log_add_msg(&game.log_ui, turn_msg);
            }
        }
        else if (!game.game_over)
        {
            mp_handle_opponent_turn(&ui_mgr, &game, &input_handler);
        }

        if (game.game_over)
        {
            if (mp_handle_game_over_input(&ui_mgr, &game, &input_handler))
            {
                game_running = false;
            }
        }
    }

    mp_cleanup_game(&ui_mgr, &game, &input_handler);
    return 0;
}
