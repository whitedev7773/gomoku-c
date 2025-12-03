#include "multiplayer.h"
#include "board.h"
#include "game_logic.h"
#include "turn_manager.h"
#include "game_logger.h"
#include "command.h"
#include "../ui/ui_manager.h"
#include "../ui/board_ui.h"
#include "../ui/input_handler.h"
#include "../ui/game_info_ui.h"
#include "../ui/log_ui.h"
#include "../ui/chat_ui.h"
#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>

// 플레이어 정보
typedef struct {
    char name[MAX_PLAYER_NAME];
    Stone color;
} PlayerData;

// 멀티플레이 게임 상태
typedef struct {
    Board board;
    BoardCursor my_cursor;
    BoardCursor opponent_cursor;
    TurnManager turn_mgr;
    GameInfoUI info_ui;
    LogUI log_ui;
    ChatUI chat_ui;
    GameLogger logger;

    PlayerData me;
    PlayerData opponent;

    NetworkManager network;
    bool waiting_for_opponent;
    bool game_over;
    GameResult result;
} MultiplayerGame;

// 프로토타입 선언
static bool mp_init_ui(UIManager *ui_mgr);
static bool mp_send_player_info(MultiplayerGame *game);
static bool mp_handle_network_messages(MultiplayerGame *game);
static void mp_render_game(UIManager *ui_mgr, MultiplayerGame *game);
static bool mp_handle_my_turn(UIManager *ui_mgr, MultiplayerGame *game);
static void mp_send_game_state_to_spectator(MultiplayerGame *game, int spectator_index);
static void mp_handle_spectator_connections(MultiplayerGame *game);
static void mp_broadcast_move_to_spectators(MultiplayerGame *game, const Message *move_msg);
static void mp_broadcast_chat_to_spectators(MultiplayerGame *game, const Message *chat_msg);

int multiplayer_run_host(int port) {
    if (port == 0) port = DEFAULT_PORT;

    // Locale 설정
    setlocale(LC_ALL, "");

    MultiplayerGame game;
    memset(&game, 0, sizeof(game));

    // 네트워크 초기화
    if (!network_init_server(&game.network, port)) {
        printf("Failed to initialize server\n");
        return -1;
    }

    if (!network_server_start_listen(&game.network)) {
        printf("Failed to start listening\n");
        network_cleanup(&game.network);
        return -1;
    }

    printf("=== GOMOKU MULTIPLAYER - HOST ===\n");
    printf("Server started on %s:%d\n", game.network.local_ip, port);
    printf("Waiting for client to connect...\n");
    printf("(Press Ctrl+C to cancel)\n\n");

    // 논블로킹 모드로 설정
    network_set_nonblocking(game.network.socket_fd, true);

    // 클라이언트 연결 대기
    while (!network_server_accept_client(&game.network)) {
        usleep(100000);  // 100ms
    }

    printf("Client connected from %s:%d\n", game.network.remote_ip, game.network.remote_port);
    printf("Starting game...\n");
    sleep(1);

    // 플레이어 정보 설정
    printf("Enter your name (max 8 chars): ");
    fgets(game.me.name, sizeof(game.me.name), stdin);
    game.me.name[strcspn(game.me.name, "\n")] = '\0';
    if (strlen(game.me.name) == 0) {
        strcpy(game.me.name, "Host");
    }
    game.me.color = BLACK;  // 호스트는 항상 BLACK (선공)

    strcpy(game.opponent.name, "Client");
    game.opponent.color = WHITE;

    // 연결 승인 메시지 전송
    Message msg;
    protocol_init_message(&msg, MSG_CONNECT_ACK, game.network.sequence_number++);
    msg.payload.connect_ack.your_color = WHITE;
    strncpy(msg.payload.connect_ack.opponent_name, game.me.name, MAX_PLAYER_NAME);
    network_send_message(&game.network, &msg);

    // 상대방 이름 수신 대기
    int attempts = 0;
    while (attempts < 50) {  // 5초 대기
        Message recv_msg;
        int result = network_receive_message(&game.network, &recv_msg);
        if (result > 0 && recv_msg.header.type == MSG_PLAYER_INFO) {
            strncpy(game.opponent.name, recv_msg.payload.player_info.name, MAX_PLAYER_NAME);
            break;
        }
        usleep(100000);
        attempts++;
    }

    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    wresize(stdscr, 30, 100);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_BLACK, COLOR_WHITE);
        init_pair(3, COLOR_RED, COLOR_BLACK);
        init_pair(4, COLOR_GREEN, COLOR_BLACK);
        init_pair(5, COLOR_YELLOW, COLOR_BLACK);
        init_pair(6, COLOR_BLUE, COLOR_BLACK);
        init_pair(7, COLOR_CYAN, COLOR_BLACK);
        init_pair(8, COLOR_WHITE, COLOR_BLACK);
    }

    UIManager ui_mgr;
    if (!ui_manager_init(&ui_mgr)) {
        endwin();
        printf("Failed to initialize UI\n");
        network_cleanup(&game.network);
        return -1;
    }

    // 게임 컴포넌트 초기화
    board_init(&game.board);
    board_ui_init_cursor(&game.my_cursor);
    board_ui_init_cursor(&game.opponent_cursor);
    turn_manager_init(&game.turn_mgr, BLACK);
    game_info_ui_init(&game.info_ui);
    log_ui_init(&game.log_ui);
    chat_ui_init(&game.chat_ui);
    logger_init(&game.logger);

    char start_msg[128];
    snprintf(start_msg, sizeof(start_msg), "Game started! You: %s (BLACK), Opponent: %s (WHITE)",
             game.me.name, game.opponent.name);
    log_ui_add_message(&game.log_ui, start_msg);
    chat_ui_add_message(&game.chat_ui, start_msg, CHAT_MSG_SYSTEM);

    // 게임 시작 메시지 전송
    protocol_init_message(&msg, MSG_GAME_START, game.network.sequence_number++);
    msg.payload.game_start.your_turn = 0;  // 클라이언트는 WHITE (후공)
    network_send_message(&game.network, &msg);

    keypad(ui_mgr.board_win, TRUE);
    wtimeout(ui_mgr.board_win, 50);

    bool game_running = true;

    while (game_running) {
        // 관전자 연결 처리
        mp_handle_spectator_connections(&game);

        // 네트워크 메시지 처리
        mp_handle_network_messages(&game);

        // UI 렌더링
        mp_render_game(&ui_mgr, &game);

        // 게임 종료 체크
        if (!game.game_over) {
            game.result = game_check_winner(&game.board);
            if (game.result != GAME_ONGOING) {
                game.game_over = true;
                char result_msg[128];
                if (game.result == GAME_BLACK_WIN) {
                    snprintf(result_msg, sizeof(result_msg), "%s (BLACK) WINS!", game.me.name);
                } else if (game.result == GAME_WHITE_WIN) {
                    snprintf(result_msg, sizeof(result_msg), "%s (WHITE) WINS!", game.opponent.name);
                } else {
                    snprintf(result_msg, sizeof(result_msg), "Game ended in a DRAW!");
                }
                log_ui_add_message(&game.log_ui, result_msg);
                log_ui_add_message(&game.log_ui, "Press 'q' to quit");
                logger_close(&game.logger);
            }
        }

        // 내 턴 처리
        Stone current_player = turn_manager_get_current_player(&game.turn_mgr);
        if (!game.game_over && current_player == game.me.color) {
            if (mp_handle_my_turn(&ui_mgr, &game)) {
                // 수를 뒀으므로 턴 변경
                turn_manager_next_turn(&game.turn_mgr);
            }
        }

        // 게임 종료 후
        if (game.game_over) {
            int ch = wgetch(ui_mgr.board_win);
            if (ch == 'q' || ch == 'Q') {
                game_running = false;
            }
        }
    }

    // 정리
    logger_close(&game.logger);
    ui_manager_cleanup(&ui_mgr);
    endwin();
    network_cleanup(&game.network);

    return 0;
}

int multiplayer_run_client(const char *server_ip, int port) {
    // 클라이언트 구현은 호스트와 유사하나 네트워크 초기화가 다름
    setlocale(LC_ALL, "");

    MultiplayerGame game;
    memset(&game, 0, sizeof(game));

    // 네트워크 초기화
    if (!network_init_client(&game.network)) {
        printf("Failed to initialize client\n");
        return -1;
    }

    printf("=== GOMOKU MULTIPLAYER - CLIENT ===\n");
    printf("Connecting to %s:%d...\n", server_ip, port);

    if (!network_client_connect(&game.network, server_ip, port)) {
        printf("Failed to connect to server\n");
        network_cleanup(&game.network);
        return -1;
    }

    printf("Connected to server!\n");

    // 플레이어 이름 입력
    printf("Enter your name (max 8 chars): ");
    fgets(game.me.name, sizeof(game.me.name), stdin);
    game.me.name[strcspn(game.me.name, "\n")] = '\0';
    if (strlen(game.me.name) == 0) {
        strcpy(game.me.name, "Client");
    }

    // 연결 승인 대기
    Message msg;
    int attempts = 0;
    bool connected = false;
    while (attempts < 50) {
        int result = network_receive_message(&game.network, &msg);
        if (result > 0 && msg.header.type == MSG_CONNECT_ACK) {
            game.me.color = msg.payload.connect_ack.your_color;
            strncpy(game.opponent.name, msg.payload.connect_ack.opponent_name, MAX_PLAYER_NAME);
            game.opponent.color = (game.me.color == BLACK) ? WHITE : BLACK;
            connected = true;
            break;
        }
        usleep(100000);
        attempts++;
    }

    if (!connected) {
        printf("Connection timeout\n");
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
    while (attempts < 50) {
        int result = network_receive_message(&game.network, &msg);
        if (result > 0 && msg.header.type == MSG_GAME_START) {
            break;
        }
        usleep(100000);
        attempts++;
    }

    // ncurses 및 게임 초기화 (호스트와 동일)
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    wresize(stdscr, 30, 100);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_BLACK, COLOR_WHITE);
        init_pair(3, COLOR_RED, COLOR_BLACK);
        init_pair(4, COLOR_GREEN, COLOR_BLACK);
        init_pair(5, COLOR_YELLOW, COLOR_BLACK);
        init_pair(6, COLOR_BLUE, COLOR_BLACK);
        init_pair(7, COLOR_CYAN, COLOR_BLACK);
        init_pair(8, COLOR_WHITE, COLOR_BLACK);
    }

    UIManager ui_mgr;
    if (!ui_manager_init(&ui_mgr)) {
        endwin();
        network_cleanup(&game.network);
        return -1;
    }

    board_init(&game.board);
    board_ui_init_cursor(&game.my_cursor);
    board_ui_init_cursor(&game.opponent_cursor);
    turn_manager_init(&game.turn_mgr, BLACK);
    game_info_ui_init(&game.info_ui);
    log_ui_init(&game.log_ui);
    chat_ui_init(&game.chat_ui);
    logger_init(&game.logger);

    char start_msg[128];
    snprintf(start_msg, sizeof(start_msg), "Game started! You: %s (%s), Opponent: %s (%s)",
             game.me.name, game.me.color == BLACK ? "BLACK" : "WHITE",
             game.opponent.name, game.opponent.color == BLACK ? "BLACK" : "WHITE");
    log_ui_add_message(&game.log_ui, start_msg);
    chat_ui_add_message(&game.chat_ui, start_msg, CHAT_MSG_SYSTEM);

    keypad(ui_mgr.board_win, TRUE);
    wtimeout(ui_mgr.board_win, 50);

    bool game_running = true;

    while (game_running) {
        mp_handle_network_messages(&game);
        mp_render_game(&ui_mgr, &game);

        if (!game.game_over) {
            game.result = game_check_winner(&game.board);
            if (game.result != GAME_ONGOING) {
                game.game_over = true;
                char result_msg[128];
                if ((game.result == GAME_BLACK_WIN && game.me.color == BLACK) ||
                    (game.result == GAME_WHITE_WIN && game.me.color == WHITE)) {
                    snprintf(result_msg, sizeof(result_msg), "You WIN!");
                } else if (game.result != GAME_DRAW) {
                    snprintf(result_msg, sizeof(result_msg), "You LOSE!");
                } else {
                    snprintf(result_msg, sizeof(result_msg), "DRAW!");
                }
                log_ui_add_message(&game.log_ui, result_msg);
                log_ui_add_message(&game.log_ui, "Press 'q' to quit");
                logger_close(&game.logger);
            }
        }

        Stone current_player = turn_manager_get_current_player(&game.turn_mgr);
        if (!game.game_over && current_player == game.me.color) {
            if (mp_handle_my_turn(&ui_mgr, &game)) {
                turn_manager_next_turn(&game.turn_mgr);
            }
        }

        if (game.game_over) {
            int ch = wgetch(ui_mgr.board_win);
            if (ch == 'q' || ch == 'Q') {
                game_running = false;
            }
        }
    }

    logger_close(&game.logger);
    ui_manager_cleanup(&ui_mgr);
    endwin();
    network_cleanup(&game.network);

    return 0;
}

// 네트워크 메시지 처리
static bool mp_handle_network_messages(MultiplayerGame *game) {
    Message msg;
    int result = network_receive_message(&game->network, &msg);

    if (result <= 0) {
        return false;
    }

    switch (msg.header.type) {
        case MSG_MOVE:
            // 상대방의 수
            if (board_place_stone(&game->board, msg.payload.move.row, msg.payload.move.col,
                                 msg.payload.move.stone)) {
                char move_msg[128];
                snprintf(move_msg, sizeof(move_msg), "%s placed at %c%02d",
                        game->opponent.name,
                        board_ui_col_to_char(msg.payload.move.col),
                        msg.payload.move.row + 1);
                log_ui_add_message(&game->log_ui, move_msg);

                logger_log_move(&game->logger, msg.payload.move.stone,
                               msg.payload.move.row, msg.payload.move.col,
                               board_get_move_count(&game->board));

                turn_manager_next_turn(&game->turn_mgr);

                // 관전자들에게 브로드캐스트
                mp_broadcast_move_to_spectators(game, &msg);
            }
            break;

        case MSG_CURSOR_UPDATE:
            game->opponent_cursor.cursor_row = msg.payload.cursor.row;
            game->opponent_cursor.cursor_col = msg.payload.cursor.col;
            break;

        case MSG_CHAT:
            // 채팅 메시지 수신
            chat_ui_add_message(&game->chat_ui, msg.payload.chat.message, CHAT_MSG_OPPONENT);

            // 관전자들에게 브로드캐스트
            mp_broadcast_chat_to_spectators(game, &msg);
            break;

        case MSG_COMMAND:
            // 명령어 수신 (상대방이 보낸 명령어)
            {
                CommandType cmd_type = msg.payload.command.command_type;
                char sys_msg[128];
                snprintf(sys_msg, sizeof(sys_msg), "Opponent sent: %s",
                        command_type_to_string(cmd_type));
                chat_ui_add_message(&game->chat_ui, sys_msg, CHAT_MSG_SYSTEM);

                // TODO: 명령어 처리 로직 구현
                // /quit, /undo, /giveup, /swap 등
            }
            break;

        default:
            break;
    }

    return true;
}

// UI 렌더링
static void mp_render_game(UIManager *ui_mgr, MultiplayerGame *game) {
    ui_manager_clear_all(ui_mgr);

    board_ui_render(ui_mgr->board_win, &game->board, &game->my_cursor);
    game_info_ui_render_bottom(ui_mgr->bottom_win, &game->board, &game->turn_mgr, &game->info_ui);

    // 채팅 UI 렌더링
    chat_ui_render(ui_mgr->chat_win, &game->chat_ui);

    // 채팅 입력 렌더링
    chat_ui_render_input(ui_mgr->chat_input_win, &game->chat_ui, 1, 1);

    Stone current_player = turn_manager_get_current_player(&game->turn_mgr);
    const char *turn_name = (current_player == game->me.color) ? "Your Turn" : "Opponent's Turn";
    mvwprintw(ui_mgr->info_win, 0, 0, "%s", turn_name);

    ui_manager_refresh_all(ui_mgr);
}

// 내 턴 처리
static bool mp_handle_my_turn(UIManager *ui_mgr, MultiplayerGame *game) {
    // 채팅 모드 확인
    if (chat_ui_is_input_mode(&game->chat_ui)) {
        int ch = wgetch(ui_mgr->board_win);
        if (ch != ERR) {
            if (ch == '\n' || ch == KEY_ENTER) {
                // 메시지 전송
                const char *msg_text = chat_ui_get_message(&game->chat_ui);
                if (strlen(msg_text) > 0) {
                    // 명령어 체크
                    if (command_is_command(msg_text)) {
                        CommandResult cmd = command_parse(msg_text);
                        if (cmd.valid) {
                            // 명령어 전송
                            Message msg;
                            protocol_init_message(&msg, MSG_COMMAND, game->network.sequence_number++);
                            msg.payload.command.command_type = cmd.type;
                            network_send_message(&game->network, &msg);

                            char sys_msg[128];
                            snprintf(sys_msg, sizeof(sys_msg), "Command sent: %s", msg_text);
                            chat_ui_add_message(&game->chat_ui, sys_msg, CHAT_MSG_SYSTEM);
                        } else {
                            chat_ui_add_message(&game->chat_ui, cmd.error_message, CHAT_MSG_SYSTEM);
                        }
                    } else {
                        // 일반 채팅 전송
                        Message msg;
                        protocol_init_message(&msg, MSG_CHAT, game->network.sequence_number++);
                        strncpy(msg.payload.chat.message, msg_text, sizeof(msg.payload.chat.message) - 1);
                        network_send_message(&game->network, &msg);

                        // 관전자들에게도 브로드캐스트
                        mp_broadcast_chat_to_spectators(game, &msg);

                        chat_ui_add_message(&game->chat_ui, msg_text, CHAT_MSG_USER);
                    }
                }
                chat_ui_exit_input_mode(&game->chat_ui);
            } else {
                chat_ui_handle_input(&game->chat_ui, ch);
            }
        }
        return false;
    }

    InputEvent event = input_get_event(ui_mgr->board_win);

    if (event.action == INPUT_NONE) {
        return false;
    }

    // Enter 또는 T 키로 채팅 모드 진입
    if (event.key_code == '\n' || event.key_code == 't' || event.key_code == 'T') {
        chat_ui_enter_input_mode(&game->chat_ui);
        return false;
    }

    switch (event.action) {
        case INPUT_MOVE_UP:
            board_ui_move_cursor(&game->my_cursor, -1, 0);
            // 커서 위치 전송
            {
                Message msg;
                protocol_init_message(&msg, MSG_CURSOR_UPDATE, game->network.sequence_number++);
                msg.payload.cursor.row = game->my_cursor.cursor_row;
                msg.payload.cursor.col = game->my_cursor.cursor_col;
                network_send_message(&game->network, &msg);
            }
            break;
        case INPUT_MOVE_DOWN:
            board_ui_move_cursor(&game->my_cursor, 1, 0);
            {
                Message msg;
                protocol_init_message(&msg, MSG_CURSOR_UPDATE, game->network.sequence_number++);
                msg.payload.cursor.row = game->my_cursor.cursor_row;
                msg.payload.cursor.col = game->my_cursor.cursor_col;
                network_send_message(&game->network, &msg);
            }
            break;
        case INPUT_MOVE_LEFT:
            board_ui_move_cursor(&game->my_cursor, 0, -1);
            {
                Message msg;
                protocol_init_message(&msg, MSG_CURSOR_UPDATE, game->network.sequence_number++);
                msg.payload.cursor.row = game->my_cursor.cursor_row;
                msg.payload.cursor.col = game->my_cursor.cursor_col;
                network_send_message(&game->network, &msg);
            }
            break;
        case INPUT_MOVE_RIGHT:
            board_ui_move_cursor(&game->my_cursor, 0, 1);
            {
                Message msg;
                protocol_init_message(&msg, MSG_CURSOR_UPDATE, game->network.sequence_number++);
                msg.payload.cursor.row = game->my_cursor.cursor_row;
                msg.payload.cursor.col = game->my_cursor.cursor_col;
                network_send_message(&game->network, &msg);
            }
            break;
        case INPUT_PLACE_STONE:
            if (board_is_empty(&game->board, game->my_cursor.cursor_row, game->my_cursor.cursor_col)) {
                if (board_place_stone(&game->board, game->my_cursor.cursor_row,
                                     game->my_cursor.cursor_col, game->me.color)) {
                    char move_msg[128];
                    snprintf(move_msg, sizeof(move_msg), "You placed at %c%02d",
                            board_ui_col_to_char(game->my_cursor.cursor_col),
                            game->my_cursor.cursor_row + 1);
                    log_ui_add_message(&game->log_ui, move_msg);

                    logger_log_move(&game->logger, game->me.color,
                                   game->my_cursor.cursor_row, game->my_cursor.cursor_col,
                                   board_get_move_count(&game->board));

                    // 네트워크로 전송
                    Message msg;
                    protocol_init_message(&msg, MSG_MOVE, game->network.sequence_number++);
                    msg.payload.move.row = game->my_cursor.cursor_row;
                    msg.payload.move.col = game->my_cursor.cursor_col;
                    msg.payload.move.stone = game->me.color;
                    network_send_message(&game->network, &msg);

                    // 관전자들에게도 브로드캐스트
                    mp_broadcast_move_to_spectators(game, &msg);

                    return true;
                }
            } else {
                log_ui_add_message(&game->log_ui, "Position already occupied!");
            }
            break;
        case INPUT_QUIT:
            // TODO: 연결 종료 메시지 전송
            break;
        default:
            break;
    }

    return false;
}

// 관전자 관련 함수들

// 관전자에게 현재 게임 상태 전송
static void mp_send_game_state_to_spectator(MultiplayerGame *game, int spectator_index) {
    Message msg;
    protocol_init_message(&msg, MSG_GAME_STATE, game->network.sequence_number++);

    // 플레이어 정보
    strncpy(msg.payload.game_state.player1_name, game->me.name, MAX_PLAYER_NAME);
    strncpy(msg.payload.game_state.player2_name, game->opponent.name, MAX_PLAYER_NAME);

    // 현재 턴
    msg.payload.game_state.current_turn = turn_manager_get_current_player(&game->turn_mgr);

    // 수 개수
    msg.payload.game_state.move_count = board_get_move_count(&game->board);

    // 보드 상태
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            msg.payload.game_state.board_state[row * BOARD_SIZE + col] = game->board.cells[row][col];
        }
    }

    network_send_to_spectator(&game->network, spectator_index, &msg);
}

// 새로운 관전자 연결 처리
static void mp_handle_spectator_connections(MultiplayerGame *game) {
    // 관전자 연결 시도 체크
    if (network_server_accept_spectator(&game->network)) {
        // 새로운 관전자 연결됨
        int new_spectator_index = game->network.spectator_count - 1;

        // 관전자로부터 MSG_SPECTATOR_CONNECT 대기
        uint8_t temp_buffer[1024];
        int spectator_fd = game->network.spectator_fds[new_spectator_index];

        // 논블로킹 recv 시도
        ssize_t received = recv(spectator_fd, temp_buffer, sizeof(temp_buffer), MSG_DONTWAIT);
        if (received > 0) {
            Message connect_msg;
            int result = protocol_deserialize(&connect_msg, temp_buffer, received);

            if (result > 0 && connect_msg.header.type == MSG_SPECTATOR_CONNECT) {
                // 관전자 이름 저장
                strncpy(game->network.spectator_names[new_spectator_index],
                       connect_msg.payload.spectator_connect.spectator_name,
                       MAX_PLAYER_NAME);

                // 승인 메시지 전송
                Message ack_msg;
                protocol_init_message(&ack_msg, MSG_SPECTATOR_CONNECT_ACK, game->network.sequence_number++);
                ack_msg.payload.spectator_connect_ack.accepted = 1;
                ack_msg.payload.spectator_connect_ack.error_code = ERR_NONE;
                ack_msg.payload.spectator_connect_ack.spectator_count = game->network.spectator_count;
                ack_msg.payload.spectator_connect_ack.max_spectators = MAX_SPECTATORS;
                network_send_to_spectator(&game->network, new_spectator_index, &ack_msg);

                // 현재 게임 상태 전송
                mp_send_game_state_to_spectator(game, new_spectator_index);

                // 다른 사람들에게 관전자 입장 알림
                Message join_msg;
                protocol_init_message(&join_msg, MSG_SPECTATOR_JOIN, game->network.sequence_number++);
                strncpy(join_msg.payload.spectator_join_leave.spectator_name,
                       game->network.spectator_names[new_spectator_index],
                       MAX_PLAYER_NAME);
                join_msg.payload.spectator_join_leave.spectator_count = game->network.spectator_count;

                // 플레이어들과 다른 관전자들에게 알림
                network_send_message(&game->network, &join_msg);
                for (int i = 0; i < MAX_SPECTATORS; i++) {
                    if (i != new_spectator_index && game->network.spectator_fds[i] >= 0) {
                        network_send_to_spectator(&game->network, i, &join_msg);
                    }
                }
            }
        }
    }
}

// 모든 관전자에게 메시지 브로드캐스트
static void mp_broadcast_move_to_spectators(MultiplayerGame *game, const Message *move_msg) {
    network_broadcast_to_spectators(&game->network, move_msg);
}

static void mp_broadcast_chat_to_spectators(MultiplayerGame *game, const Message *chat_msg) {
    network_broadcast_to_spectators(&game->network, chat_msg);
}
