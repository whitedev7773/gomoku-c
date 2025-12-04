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
#include "../ui/modal_ui.h"
#include "../ui/theme.h"
#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>

// 플레이어 정보
typedef struct
{
    char name[MAX_PLAYER_NAME];
    Stone color;
} PlayerData;

// 멀티플레이 게임 상태
typedef struct
{
    Board board;
    BoardCursor my_cursor;
    BoardCursor opponent_cursor;
    TurnManager turn_mgr;
    GameInfoUI info_ui;
    LogUI log_ui;
    ChatUI chat_ui;
    ModalUI modal_ui;
    GameLogger logger;

    PlayerData me;
    PlayerData opponent;

    NetworkManager network;
    bool waiting_for_opponent;
    bool game_over;
    bool quit_requested; // /quit 명령어로 메인 화면 복귀 요청
    GameResult result;
    bool swap_used;    // Swap Rule이 이미 사용되었는지 추적
    bool first_render; // Dirty flag 기반 렌더링을 위한 변수
} MultiplayerGame;

// 프로토타입 선언
static bool mp_init_ui(UIManager *ui_mgr);
static bool mp_send_player_info(MultiplayerGame *game);
static bool mp_handle_network_messages(UIManager *ui_mgr, MultiplayerGame *game);
static void mp_render_game(UIManager *ui_mgr, MultiplayerGame *game);
static bool mp_handle_my_turn(UIManager *ui_mgr, MultiplayerGame *game, InputHandler *input_handler);
static void mp_handle_opponent_turn(UIManager *ui_mgr, MultiplayerGame *game, InputHandler *input_handler);
static void mp_send_game_state_to_spectator(MultiplayerGame *game, int spectator_index);
static void mp_handle_spectator_connections(MultiplayerGame *game);
static void mp_broadcast_move_to_spectators(MultiplayerGame *game, const Message *move_msg);
static void mp_broadcast_chat_to_spectators(MultiplayerGame *game, const Message *chat_msg);
static bool mp_send_with_error_check(MultiplayerGame *game, const Message *msg, const char *error_context);

int multiplayer_run_host(int port, GameRule rule)
{
    if (port == 0)
        port = DEFAULT_PORT;

    // Locale 설정
    setlocale(LC_ALL, "");

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

    printf("=== GOMOKU MULTIPLAYER - HOST ===\n");
    printf("Server started on %s:%d\n", game.network.local_ip, port);
    printf("Waiting for client to connect...\n");
    printf("(Press Ctrl+C to cancel)\n\n");

    // 논블로킹 모드로 설정
    network_set_nonblocking(game.network.socket_fd, true);

    // 클라이언트 연결 대기
    while (!network_server_accept_client(&game.network))
    {
        usleep(100000); // 100ms
    }

    printf("Client connected from %s:%d\n", game.network.remote_ip, game.network.remote_port);
    printf("Starting game...\n");
    sleep(1);

    // 플레이어 정보 설정
    printf("Enter your name (max 8 chars): ");
    fgets(game.me.name, sizeof(game.me.name), stdin);
    game.me.name[strcspn(game.me.name, "\n")] = '\0';
    if (strlen(game.me.name) == 0)
    {
        strcpy(game.me.name, "Host");
    }
    game.me.color = BLACK; // 호스트는 항상 BLACK (선공)

    strcpy(game.opponent.name, "Client");
    game.opponent.color = WHITE;

    // 연결 승인 메시지 전송 (규칙 정보 포함)
    Message msg;
    protocol_init_message(&msg, MSG_CONNECT_ACK, game.network.sequence_number++);
    msg.payload.connect_ack.your_color = WHITE;
    msg.payload.connect_ack.game_rule = rule;
    strncpy(msg.payload.connect_ack.opponent_name, game.me.name, MAX_PLAYER_NAME);
    network_send_message(&game.network, &msg);

    // 상대방 이름 수신 대기
    int attempts = 0;
    while (attempts < 50)
    { // 5초 대기
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

    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    wresize(stdscr, 30, 100);

    // 테마 초기화
    theme_init(theme_get_current());

    UIManager ui_mgr;
    if (!ui_manager_init(&ui_mgr))
    {
        endwin();
        printf("Failed to initialize UI\n");
        network_cleanup(&game.network);
        return -1;
    }

    // 게임 컴포넌트 초기화
    board_init_with_rule(&game.board, rule);
    board_ui_init_cursor(&game.my_cursor);
    board_ui_init_cursor(&game.opponent_cursor);
    turn_manager_init(&game.turn_mgr, BLACK);
    game_info_ui_init(&game.info_ui);
    log_ui_init(&game.log_ui);
    chat_ui_init(&game.chat_ui);
    modal_ui_init(&game.modal_ui);

    // 로거 초기화 및 에러 체크
    if (!logger_init(&game.logger))
    {
        char error_msg[MODAL_MAX_MESSAGE_LENGTH];
        snprintf(error_msg, sizeof(error_msg),
                 "Failed to create game log file. The game will continue without logging.");
        modal_ui_show(&game.modal_ui, MODAL_ERROR, error_msg);
        log_ui_add_message(&game.log_ui, "Warning: Game logging disabled");
    }

    char start_msg[128];
    snprintf(start_msg, sizeof(start_msg), "Game started! You: %s (BLACK), Opponent: %s (WHITE)",
             game.me.name, game.opponent.name);
    log_ui_add_message(&game.log_ui, start_msg);
    chat_ui_add_message(&game.chat_ui, start_msg, CHAT_MSG_SYSTEM);

    // 게임패드 입력 핸들러 초기화
    InputHandler input_handler;
    input_handler_init(&input_handler);

    // 게임 시작 메시지 전송
    protocol_init_message(&msg, MSG_GAME_START, game.network.sequence_number++);
    msg.payload.game_start.your_turn = 0; // 클라이언트는 WHITE (후공)
    network_send_message(&game.network, &msg);

    keypad(ui_mgr.board_win, TRUE);
    wtimeout(ui_mgr.board_win, 50);

    bool game_running = true;
    game.first_render = true; // Dirty flag 기반 렌더링 초기화

    while (game_running)
    {
        // 관전자 연결 처리
        mp_handle_spectator_connections(&game);

        // 네트워크 메시지 처리
        mp_handle_network_messages(&ui_mgr, &game);

        // UI 렌더링
        mp_render_game(&ui_mgr, &game);

        // 게임 종료 체크
        if (!game.game_over)
        {
            game.result = game_check_winner(&game.board);
            if (game.result != GAME_ONGOING)
            {
                game.game_over = true;
                char result_msg[128];
                if (game.result == GAME_BLACK_WIN)
                {
                    snprintf(result_msg, sizeof(result_msg), "%s (BLACK) WINS!", game.me.name);
                }
                else if (game.result == GAME_WHITE_WIN)
                {
                    snprintf(result_msg, sizeof(result_msg), "%s (WHITE) WINS!", game.opponent.name);
                }
                else
                {
                    snprintf(result_msg, sizeof(result_msg), "Game ended in a DRAW!");
                }
                log_ui_add_message(&game.log_ui, result_msg);
                log_ui_add_message(&game.log_ui, "Press 'q' to quit");

                // 게임 결과 모달 표시
                modal_ui_show(&game.modal_ui, MODAL_GAME_RESULT, result_msg);

                logger_close(&game.logger);
            }
        }

        // 내 턴 처리
        Stone current_player = turn_manager_get_current_player(&game.turn_mgr);
        if (!game.game_over && current_player == game.me.color)
        {
            if (mp_handle_my_turn(&ui_mgr, &game, &input_handler))
            {
                // 수를 뒀으므로 턴 변경
                turn_manager_next_turn(&game.turn_mgr);
            }
        }
        else if (!game.game_over)
        {
            // 상대 턴일 때도 채팅/모달 입력 처리
            mp_handle_opponent_turn(&ui_mgr, &game, &input_handler);
        }

        // 게임 종료 후
        if (game.game_over)
        {
            // /quit 명령어로 종료한 경우 바로 메인 화면으로
            if (game.quit_requested)
            {
                game_running = false;
                continue;
            }

            InputEvent event = input_handler_get_event(&input_handler, ui_mgr.board_win);
            int ch = event.key_code;

            // 모달이 활성화되어 있으면 모달 입력 처리
            if (modal_ui_is_active(&game.modal_ui) && ch != ERR)
            {
                ModalResult result = modal_ui_handle_input(&game.modal_ui, ch);
                if (result == MODAL_RESULT_OK || result == MODAL_RESULT_CANCEL)
                {
                    modal_ui_close(&game.modal_ui);
                    game_running = false; // 메인 화면으로 이동
                }
            }

            if (event.action == INPUT_QUIT)
            {
                game_running = false;
            }
        }
    }

    // 정리
    input_handler_cleanup(&input_handler);
    logger_close(&game.logger);
    ui_manager_cleanup(&ui_mgr);
    endwin();
    network_cleanup(&game.network);

    return 0;
}

int multiplayer_run_client(const char *server_ip, int port, GameRule rule)
{
    // 클라이언트 구현은 호스트와 유사하나 네트워크 초기화가 다름
    setlocale(LC_ALL, "");

    MultiplayerGame game;
    memset(&game, 0, sizeof(game));
    GameRule received_rule = rule; // 호스트로부터 받을 규칙 저장용

    // 네트워크 초기화
    if (!network_init_client(&game.network))
    {
        printf("Failed to initialize client\n");
        return -1;
    }

    printf("=== GOMOKU MULTIPLAYER - CLIENT ===\n");
    printf("Connecting to %s:%d...\n", server_ip, port);

    if (!network_client_connect(&game.network, server_ip, port))
    {
        printf("Failed to connect to server\n");
        network_cleanup(&game.network);
        return -1;
    }

    printf("Connected to server!\n");

    // 플레이어 이름 입력
    printf("Enter your name (max 8 chars): ");
    fgets(game.me.name, sizeof(game.me.name), stdin);
    game.me.name[strcspn(game.me.name, "\n")] = '\0';
    if (strlen(game.me.name) == 0)
    {
        strcpy(game.me.name, "Client");
    }

    // 연결 승인 대기 (규칙 정보 포함)
    Message msg;
    int attempts = 0;
    bool connected = false;
    while (attempts < 50)
    {
        int result = network_receive_message(&game.network, &msg);
        if (result > 0 && msg.header.type == MSG_CONNECT_ACK)
        {
            game.me.color = msg.payload.connect_ack.your_color;
            received_rule = msg.payload.connect_ack.game_rule; // 호스트가 선택한 규칙 받기
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

    // ncurses 및 게임 초기화 (호스트와 동일)
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    wresize(stdscr, 30, 100);

    // 테마 초기화
    theme_init(theme_get_current());

    UIManager ui_mgr;
    if (!ui_manager_init(&ui_mgr))
    {
        endwin();
        network_cleanup(&game.network);
        return -1;
    }

    board_init_with_rule(&game.board, received_rule);
    board_ui_init_cursor(&game.my_cursor);
    board_ui_init_cursor(&game.opponent_cursor);
    turn_manager_init(&game.turn_mgr, BLACK);
    game_info_ui_init(&game.info_ui);
    log_ui_init(&game.log_ui);
    chat_ui_init(&game.chat_ui);
    modal_ui_init(&game.modal_ui);

    // 로거 초기화 및 에러 체크
    if (!logger_init(&game.logger))
    {
        char error_msg[MODAL_MAX_MESSAGE_LENGTH];
        snprintf(error_msg, sizeof(error_msg),
                 "Failed to create game log file. The game will continue without logging.");
        modal_ui_show(&game.modal_ui, MODAL_ERROR, error_msg);
        log_ui_add_message(&game.log_ui, "Warning: Game logging disabled");
    }

    char start_msg[128];
    snprintf(start_msg, sizeof(start_msg), "Game started! You: %s (%s), Opponent: %s (%s)",
             game.me.name, game.me.color == BLACK ? "BLACK" : "WHITE",
             game.opponent.name, game.opponent.color == BLACK ? "BLACK" : "WHITE");
    log_ui_add_message(&game.log_ui, start_msg);
    chat_ui_add_message(&game.chat_ui, start_msg, CHAT_MSG_SYSTEM);

    // 게임패드 입력 핸들러 초기화
    InputHandler input_handler;
    input_handler_init(&input_handler);

    keypad(ui_mgr.board_win, TRUE);
    wtimeout(ui_mgr.board_win, 50);

    bool game_running = true;
    game.first_render = true; // Dirty flag 기반 렌더링 초기화

    while (game_running)
    {
        mp_handle_network_messages(&ui_mgr, &game);
        mp_render_game(&ui_mgr, &game);

        if (!game.game_over)
        {
            game.result = game_check_winner(&game.board);
            if (game.result != GAME_ONGOING)
            {
                game.game_over = true;
                char result_msg[128];
                if ((game.result == GAME_BLACK_WIN && game.me.color == BLACK) ||
                    (game.result == GAME_WHITE_WIN && game.me.color == WHITE))
                {
                    snprintf(result_msg, sizeof(result_msg), "You WIN!");
                }
                else if (game.result != GAME_DRAW)
                {
                    snprintf(result_msg, sizeof(result_msg), "You LOSE!");
                }
                else
                {
                    snprintf(result_msg, sizeof(result_msg), "DRAW!");
                }
                log_ui_add_message(&game.log_ui, result_msg);
                log_ui_add_message(&game.log_ui, "Press 'q' to quit");

                // 게임 결과 모달 표시
                modal_ui_show(&game.modal_ui, MODAL_GAME_RESULT, result_msg);

                logger_close(&game.logger);
            }
        }

        Stone current_player = turn_manager_get_current_player(&game.turn_mgr);
        if (!game.game_over && current_player == game.me.color)
        {
            if (mp_handle_my_turn(&ui_mgr, &game, &input_handler))
            {
                turn_manager_next_turn(&game.turn_mgr);
            }
        }
        else if (!game.game_over)
        {
            // 상대 턴일 때도 채팅/모달 입력 처리
            mp_handle_opponent_turn(&ui_mgr, &game, &input_handler);
        }

        if (game.game_over)
        {
            // /quit 명령어로 종료한 경우 바로 메인 화면으로
            if (game.quit_requested)
            {
                game_running = false;
                continue;
            }

            InputEvent event = input_handler_get_event(&input_handler, ui_mgr.board_win);
            int ch = event.key_code;

            // 모달이 활성화되어 있으면 모달 입력 처리
            if (modal_ui_is_active(&game.modal_ui) && ch != ERR)
            {
                ModalResult result = modal_ui_handle_input(&game.modal_ui, ch);
                if (result == MODAL_RESULT_OK || result == MODAL_RESULT_CANCEL)
                {
                    modal_ui_close(&game.modal_ui);
                    game_running = false; // 메인 화면으로 이동
                }
            }

            if (event.action == INPUT_QUIT)
            {
                game_running = false;
            }
        }
    }

    input_handler_cleanup(&input_handler);
    logger_close(&game.logger);
    ui_manager_cleanup(&ui_mgr);
    endwin();
    network_cleanup(&game.network);

    return 0;
}

// 네트워크 메시지 처리
static bool mp_handle_network_messages(UIManager *ui_mgr, MultiplayerGame *game)
{
    Message msg;
    int result = network_receive_message(&game->network, &msg);

    // 연결 끊김 또는 에러 체크
    if (result < 0)
    {
        // 연결이 끊어졌거나 에러 발생
        if (!network_is_connected(&game->network) ||
            game->network.state == NET_DISCONNECTED ||
            game->network.state == NET_ERROR)
        {

            if (!game->game_over)
            { // 아직 게임이 끝나지 않았으면
                char modal_msg[MODAL_MAX_MESSAGE_LENGTH];
                snprintf(modal_msg, sizeof(modal_msg),
                         "Connection lost with %s. Game will end.",
                         game->opponent.name);
                modal_ui_show(&game->modal_ui, MODAL_ERROR, modal_msg);

                char log_msg[128];
                snprintf(log_msg, sizeof(log_msg), "Connection lost!");
                log_ui_add_message(&game->log_ui, log_msg);
                chat_ui_add_message(&game->chat_ui, "Connection lost", CHAT_MSG_SYSTEM);

                game->game_over = true;
            }
        }
        return false;
    }

    if (result == 0)
    {
        return false; // No data available
    }

    switch (msg.header.type)
    {
    case MSG_MOVE:
        // 상대방의 수
        if (board_place_stone(&game->board, msg.payload.move.row, msg.payload.move.col,
                              msg.payload.move.stone))
        {
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
        // 상대방 커서 업데이트 (dirty flag 설정 포함)
        board_ui_update_opponent_cursor(&game->opponent_cursor,
                                        msg.payload.cursor.row,
                                        msg.payload.cursor.col,
                                        &ui_mgr->render_flags);
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
            char modal_msg[MODAL_MAX_MESSAGE_LENGTH];

            switch (cmd_type)
            {
            case CMD_QUIT:
                // 상대방 퇴장 처리
                snprintf(modal_msg, sizeof(modal_msg), "%s has quit the game.", game->opponent.name);
                modal_ui_show(&game->modal_ui, MODAL_ERROR, modal_msg);
                game->game_over = true;
                break;

            case CMD_GIVEUP:
                // 상대방 기권
                game->game_over = true;
                game->result = (game->opponent.color == BLACK) ? GAME_WHITE_WIN : GAME_BLACK_WIN;
                snprintf(modal_msg, sizeof(modal_msg), "%s has given up!", game->opponent.name);
                modal_ui_show(&game->modal_ui, MODAL_GAME_RESULT, modal_msg);
                chat_ui_add_message(&game->chat_ui, modal_msg, CHAT_MSG_SYSTEM);
                break;

            case CMD_UNDO:
                // 무르기 요청 수신 - 항상 승인 필요
                snprintf(modal_msg, sizeof(modal_msg), "%s wants to undo the last move", game->opponent.name);
                modal_ui_show(&game->modal_ui, MODAL_UNDO_RESPONSE, modal_msg);
                break;

            case CMD_SWAP:
                // Swap 요청 수신
                snprintf(modal_msg, sizeof(modal_msg), "%s wants to swap colors", game->opponent.name);
                modal_ui_show(&game->modal_ui, MODAL_SWAP_RESPONSE, modal_msg);
                break;

            default:
                break;
            }
        }
        break;

    case MSG_COMMAND_RESPONSE:
        // 명령어 응답 수신 (Undo/Swap 수락 또는 거절)
        {
            uint8_t cmd_type = msg.payload.command_response.command_type;
            bool accepted = msg.payload.command_response.accepted;

            // 대기 모달이 있으면 닫기
            if (modal_ui_is_active(&game->modal_ui) &&
                (game->modal_ui.type == MODAL_UNDO_REQUEST || game->modal_ui.type == MODAL_SWAP_REQUEST))
            {
                modal_ui_close(&game->modal_ui);
                game->first_render = true;
            }

            if (cmd_type == CMD_UNDO)
            {
                if (accepted)
                {
                    // Undo 수락됨 - 마지막 두 수 되돌리기
                    // 턴 변경 없음 (두 수 되돌리면 턴이 그대로)
                    board_undo_last_move(&game->board);
                    board_undo_last_move(&game->board);
                    chat_ui_add_message(&game->chat_ui, "Undo accepted by opponent", CHAT_MSG_SYSTEM);
                    game->first_render = true;
                    ui_render_flags_set(&ui_mgr->render_flags, RENDER_BOARD_FULL);
                    ui_render_flags_set(&ui_mgr->render_flags, RENDER_INFO);
                }
                else
                {
                    chat_ui_add_message(&game->chat_ui, "Undo rejected by opponent", CHAT_MSG_SYSTEM);
                }
            }
            else if (cmd_type == CMD_SWAP)
            {
                if (accepted)
                {
                    // Swap 수락됨 - 색상 교환
                    Stone temp = game->me.color;
                    game->me.color = game->opponent.color;
                    game->opponent.color = temp;
                    game->swap_used = true;
                    chat_ui_add_message(&game->chat_ui, "Swap accepted by opponent", CHAT_MSG_SYSTEM);
                    game->first_render = true;
                    ui_render_flags_set(&ui_mgr->render_flags, RENDER_INFO);
                }
                else
                {
                    chat_ui_add_message(&game->chat_ui, "Swap rejected by opponent", CHAT_MSG_SYSTEM);
                }
            }
        }
        break;

    default:
        break;
    }

    return true;
}

// UI 렌더링 (Dirty Flag 기반 선택적 렌더링)
static void mp_render_game(UIManager *ui_mgr, MultiplayerGame *game)
{
    // 현재 플레이어에 따라 금수 마크 업데이트 (Renju Rule)
    Stone current_player = turn_manager_get_current_player(&game->turn_mgr);
    if (current_player == BLACK)
    {
        board_update_forbidden_marks(&game->board, BLACK);
    }

    UIRenderFlags *render_flags = &ui_mgr->render_flags;

    // 타이머와 플레이 시간은 항상 dirty로 설정 (매 루프 체크)
    ui_render_flags_set(render_flags, RENDER_TIMER);
    ui_render_flags_set(render_flags, RENDER_PLAY_TIME);

    // 보드 렌더링 (멀티플레이용 - 상대방 커서 포함)
    board_ui_selective_render_multiplayer(ui_mgr->board_win, &game->board,
                                          &game->my_cursor, &game->opponent_cursor,
                                          render_flags, game->first_render);

    // 게임 정보 렌더링 (하단, 선택적)
    game_info_ui_selective_render(ui_mgr->bottom_win, &game->board, &game->turn_mgr,
                                  &game->info_ui, render_flags, game->first_render);

    // 채팅 UI 렌더링 (선택적)
    chat_ui_selective_render(ui_mgr->chat_win, &game->chat_ui, render_flags, game->first_render);

    // 채팅 입력 렌더링 (선택적)
    chat_ui_selective_render_input(ui_mgr->chat_input_win, &game->chat_ui,
                                   render_flags, game->first_render, 1, 1);

    // 우측 info 창 (현재 턴 표시) - 턴 변경 시에만
    if (game->first_render || ui_render_flags_is_set(render_flags, RENDER_INFO))
    {
        const char *turn_name = (current_player == game->me.color) ? "Your Turn" : "Opponent's Turn";
        mvwprintw(ui_mgr->info_win, 0, 0, "%-20s", turn_name);
        wrefresh(ui_mgr->info_win);
        ui_render_flags_clear(render_flags, RENDER_INFO);
    }

    // 첫 렌더링 완료
    game->first_render = false;

    // 모달이 활성화되어 있으면 가장 위에 렌더링
    if (modal_ui_is_active(&game->modal_ui))
    {
        modal_ui_render(stdscr, &game->modal_ui);
    }
}

// 내 턴 처리
static bool mp_handle_my_turn(UIManager *ui_mgr, MultiplayerGame *game, InputHandler *input_handler)
{
    // 모달이 활성화되어 있으면 모달 입력만 처리
    if (modal_ui_is_active(&game->modal_ui))
    {
        InputEvent event = input_handler_get_event(input_handler, ui_mgr->board_win);
        int ch = event.key_code;
        if (ch != ERR)
        {
            ModalResult result = modal_ui_handle_input(&game->modal_ui, ch);

            // 모달 결과 처리
            switch (result)
            {
            case MODAL_RESULT_YES:
                // 기권 확정
                if (game->modal_ui.type == MODAL_GIVEUP)
                {
                    game->game_over = true;
                    game->result = (game->me.color == BLACK) ? GAME_WHITE_WIN : GAME_BLACK_WIN;
                    modal_ui_close(&game->modal_ui);

                    // 기권 메시지 전송
                    Message msg;
                    protocol_init_message(&msg, MSG_COMMAND, game->network.sequence_number++);
                    msg.payload.command.command_type = CMD_GIVEUP;
                    mp_send_with_error_check(game, &msg, "sending giveup");
                }
                break;

            case MODAL_RESULT_NO:
            case MODAL_RESULT_CANCEL:
                // 모달 닫기
                modal_ui_close(&game->modal_ui);
                break;

            case MODAL_RESULT_ACCEPT:
                // 무르기/Swap 수락
                if (game->modal_ui.type == MODAL_UNDO_RESPONSE)
                {
                    // 수락 응답 전송
                    Message msg;
                    protocol_init_message(&msg, MSG_COMMAND_RESPONSE, game->network.sequence_number++);
                    msg.payload.command_response.command_type = CMD_UNDO;
                    msg.payload.command_response.accepted = 1;
                    strncpy(msg.payload.command_response.message, "Undo accepted", sizeof(msg.payload.command_response.message) - 1);
                    network_send_message(&game->network, &msg);

                    // 로컬에서도 Undo 적용 - 마지막 두 수 되돌리기
                    // 턴 변경 없음 (두 수 되돌리면 턴이 그대로)
                    board_undo_last_move(&game->board);
                    board_undo_last_move(&game->board);

                    modal_ui_close(&game->modal_ui);
                    chat_ui_add_message(&game->chat_ui, "Undo accepted", CHAT_MSG_SYSTEM);
                    game->first_render = true;
                }
                else if (game->modal_ui.type == MODAL_SWAP_RESPONSE)
                {
                    // Swap 수락 응답 전송
                    Message msg;
                    protocol_init_message(&msg, MSG_COMMAND_RESPONSE, game->network.sequence_number++);
                    msg.payload.command_response.command_type = CMD_SWAP;
                    msg.payload.command_response.accepted = 1;
                    strncpy(msg.payload.command_response.message, "Swap accepted", sizeof(msg.payload.command_response.message) - 1);
                    network_send_message(&game->network, &msg);

                    // 로컬에서도 Swap 적용
                    Stone temp = game->me.color;
                    game->me.color = game->opponent.color;
                    game->opponent.color = temp;
                    game->swap_used = true;

                    modal_ui_close(&game->modal_ui);
                    chat_ui_add_message(&game->chat_ui, "Swap accepted", CHAT_MSG_SYSTEM);
                    game->first_render = true;
                }
                break;

            case MODAL_RESULT_DECLINE:
                // 무르기/Swap 거절
                if (game->modal_ui.type == MODAL_UNDO_RESPONSE)
                {
                    // 거절 응답 전송
                    Message msg;
                    protocol_init_message(&msg, MSG_COMMAND_RESPONSE, game->network.sequence_number++);
                    msg.payload.command_response.command_type = CMD_UNDO;
                    msg.payload.command_response.accepted = 0;
                    strncpy(msg.payload.command_response.message, "Undo declined", sizeof(msg.payload.command_response.message) - 1);
                    network_send_message(&game->network, &msg);

                    chat_ui_add_message(&game->chat_ui, "Undo declined", CHAT_MSG_SYSTEM);
                }
                else if (game->modal_ui.type == MODAL_SWAP_RESPONSE)
                {
                    // 거절 응답 전송
                    Message msg;
                    protocol_init_message(&msg, MSG_COMMAND_RESPONSE, game->network.sequence_number++);
                    msg.payload.command_response.command_type = CMD_SWAP;
                    msg.payload.command_response.accepted = 0;
                    strncpy(msg.payload.command_response.message, "Swap declined", sizeof(msg.payload.command_response.message) - 1);
                    network_send_message(&game->network, &msg);

                    chat_ui_add_message(&game->chat_ui, "Swap declined", CHAT_MSG_SYSTEM);
                }
                modal_ui_close(&game->modal_ui);
                break;

            case MODAL_RESULT_OK:
                // 게임 결과 확인 또는 에러 확인
                modal_ui_close(&game->modal_ui);
                break;

            default:
                break;
            }

            // 모달이 닫혔으면 전체 재렌더링
            if (!modal_ui_is_active(&game->modal_ui))
            {
                game->first_render = true;
            }
        }
        return false;
    }

    // 채팅 모드 확인
    if (chat_ui_is_input_mode(&game->chat_ui))
    {
        InputEvent event = input_handler_get_event(input_handler, ui_mgr->board_win);
        int ch = event.key_code;
        if (ch != ERR)
        {
            if (ch == '\n' || ch == KEY_ENTER)
            {
                // 메시지 전송
                const char *msg_text = chat_ui_get_message(&game->chat_ui);
                if (strlen(msg_text) > 0)
                {
                    // 명령어 체크
                    if (command_is_command(msg_text))
                    {
                        CommandResult cmd = command_parse(msg_text);
                        if (cmd.valid)
                        {
                            char modal_msg[MODAL_MAX_MESSAGE_LENGTH];

                            // 명령어 타입에 따라 처리
                            switch (cmd.type)
                            {
                            case CMD_GIVEUP:
                                // 기권 확인 모달 표시
                                snprintf(modal_msg, sizeof(modal_msg), "Are you sure you want to give up?");
                                modal_ui_show(&game->modal_ui, MODAL_GIVEUP, modal_msg);
                                break;

                            case CMD_UNDO:
                                // 무르기 요청 - 항상 상대 승인 필요
                                {
                                    if (board_get_move_count(&game->board) == 0)
                                    {
                                        chat_ui_add_message(&game->chat_ui, "No moves to undo", CHAT_MSG_SYSTEM);
                                    }
                                    else
                                    {
                                        Message msg;
                                        protocol_init_message(&msg, MSG_COMMAND, game->network.sequence_number++);
                                        msg.payload.command.command_type = CMD_UNDO;
                                        network_send_message(&game->network, &msg);

                                        snprintf(modal_msg, sizeof(modal_msg), "Waiting for opponent's response...");
                                        modal_ui_show(&game->modal_ui, MODAL_UNDO_REQUEST, modal_msg);
                                        chat_ui_add_message(&game->chat_ui, "Undo request sent", CHAT_MSG_SYSTEM);
                                    }
                                }
                                break;

                            case CMD_SWAP:
                                // Swap 요청 전송 후 대기 모달 표시
                                {
                                    // Swap Rule 제한: 백돌만, 한 번만, 3수 이후
                                    if (game->me.color != WHITE)
                                    {
                                        chat_ui_add_message(&game->chat_ui, "Only WHITE can use swap", CHAT_MSG_SYSTEM);
                                    }
                                    else if (game->swap_used)
                                    {
                                        chat_ui_add_message(&game->chat_ui, "Swap already used", CHAT_MSG_SYSTEM);
                                    }
                                    else if (board_get_move_count(&game->board) < 3)
                                    {
                                        chat_ui_add_message(&game->chat_ui, "Swap available after 3 moves", CHAT_MSG_SYSTEM);
                                    }
                                    else
                                    {
                                        Message msg;
                                        protocol_init_message(&msg, MSG_COMMAND, game->network.sequence_number++);
                                        msg.payload.command.command_type = CMD_SWAP;
                                        network_send_message(&game->network, &msg);

                                        snprintf(modal_msg, sizeof(modal_msg), "Waiting for opponent's response...");
                                        modal_ui_show(&game->modal_ui, MODAL_SWAP_REQUEST, modal_msg);
                                        chat_ui_add_message(&game->chat_ui, "Swap request sent", CHAT_MSG_SYSTEM);
                                    }
                                }
                                break;

                            case CMD_QUIT:
                                // 퇴장 메시지 전송
                                {
                                    Message msg;
                                    protocol_init_message(&msg, MSG_COMMAND, game->network.sequence_number++);
                                    msg.payload.command.command_type = CMD_QUIT;
                                    network_send_message(&game->network, &msg);
                                    game->game_over = true;
                                    game->quit_requested = true; // 메인 화면으로 바로 이동
                                }
                                break;

                            case CMD_HELP:
                                // 도움말 출력
                                chat_ui_add_message(&game->chat_ui, "=== Commands ===", CHAT_MSG_SYSTEM);
                                chat_ui_add_message(&game->chat_ui, "/help  - Show help", CHAT_MSG_SYSTEM);
                                chat_ui_add_message(&game->chat_ui, "/quit  - Leave game", CHAT_MSG_SYSTEM);
                                chat_ui_add_message(&game->chat_ui, "/undo  - Request undo", CHAT_MSG_SYSTEM);
                                chat_ui_add_message(&game->chat_ui, "/giveup - Forfeit", CHAT_MSG_SYSTEM);
                                chat_ui_add_message(&game->chat_ui, "/swap  - Swap colors", CHAT_MSG_SYSTEM);
                                break;

                            default:
                                break;
                            }
                        }
                        else
                        {
                            chat_ui_add_message(&game->chat_ui, cmd.error_message, CHAT_MSG_SYSTEM);
                        }
                    }
                    else
                    {
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
            }
            else
            {
                chat_ui_handle_input(&game->chat_ui, ch);
            }
        }
        return false;
    }

    InputEvent event = input_handler_get_event(input_handler, ui_mgr->board_win);

    if (event.action == INPUT_NONE)
    {
        return false;
    }

    // Enter 또는 T 키로 채팅 모드 진입
    if (event.key_code == '\n' || event.key_code == 't' || event.key_code == 'T')
    {
        chat_ui_enter_input_mode(&game->chat_ui);
        return false;
    }

    switch (event.action)
    {
    case INPUT_MOVE_UP:
        board_ui_move_cursor_with_flags(&game->my_cursor, -1, 0, &ui_mgr->render_flags);
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
        board_ui_move_cursor_with_flags(&game->my_cursor, 1, 0, &ui_mgr->render_flags);
        {
            Message msg;
            protocol_init_message(&msg, MSG_CURSOR_UPDATE, game->network.sequence_number++);
            msg.payload.cursor.row = game->my_cursor.cursor_row;
            msg.payload.cursor.col = game->my_cursor.cursor_col;
            network_send_message(&game->network, &msg);
        }
        break;
    case INPUT_MOVE_LEFT:
        board_ui_move_cursor_with_flags(&game->my_cursor, 0, -1, &ui_mgr->render_flags);
        {
            Message msg;
            protocol_init_message(&msg, MSG_CURSOR_UPDATE, game->network.sequence_number++);
            msg.payload.cursor.row = game->my_cursor.cursor_row;
            msg.payload.cursor.col = game->my_cursor.cursor_col;
            network_send_message(&game->network, &msg);
        }
        break;
    case INPUT_MOVE_RIGHT:
        board_ui_move_cursor_with_flags(&game->my_cursor, 0, 1, &ui_mgr->render_flags);
        {
            Message msg;
            protocol_init_message(&msg, MSG_CURSOR_UPDATE, game->network.sequence_number++);
            msg.payload.cursor.row = game->my_cursor.cursor_row;
            msg.payload.cursor.col = game->my_cursor.cursor_col;
            network_send_message(&game->network, &msg);
        }
        break;
    case INPUT_PLACE_STONE:
        if (board_is_empty(&game->board, game->my_cursor.cursor_row, game->my_cursor.cursor_col))
        {
            // Renju Rule: 흑돌은 금수 위치에 놓을 수 없음
            if (game->me.color == BLACK && board_is_forbidden(&game->board, game->my_cursor.cursor_row, game->my_cursor.cursor_col))
            {
                chat_ui_add_message(&game->chat_ui, "Forbidden move! (Renju Rule)", CHAT_MSG_SYSTEM);
            }
            else if (board_place_stone(&game->board, game->my_cursor.cursor_row,
                                       game->my_cursor.cursor_col, game->me.color))
            {
                char move_msg[128];
                snprintf(move_msg, sizeof(move_msg), "You placed at %c%02d",
                         board_ui_col_to_char(game->my_cursor.cursor_col),
                         game->my_cursor.cursor_row + 1);
                log_ui_add_message(&game->log_ui, move_msg);

                logger_log_move(&game->logger, game->me.color,
                                game->my_cursor.cursor_row, game->my_cursor.cursor_col,
                                board_get_move_count(&game->board));

                // Dirty flag 설정
                ui_render_flags_add_dirty_cell(&ui_mgr->render_flags,
                                               game->my_cursor.cursor_row,
                                               game->my_cursor.cursor_col);
                ui_render_flags_set(&ui_mgr->render_flags, RENDER_LAST_MOVE);
                ui_render_flags_set(&ui_mgr->render_flags, RENDER_CURRENT_TURN);
                ui_render_flags_set(&ui_mgr->render_flags, RENDER_INFO);

                // 네트워크로 전송
                Message msg;
                protocol_init_message(&msg, MSG_MOVE, game->network.sequence_number++);
                msg.payload.move.row = game->my_cursor.cursor_row;
                msg.payload.move.col = game->my_cursor.cursor_col;
                msg.payload.move.stone = game->me.color;

                if (!mp_send_with_error_check(game, &msg, "sending move"))
                {
                    // Send 실패, 게임이 종료됨
                    return false;
                }

                // 관전자들에게도 브로드캐스트
                mp_broadcast_move_to_spectators(game, &msg);

                return true;
            }
        }
        else
        {
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

// 상대 턴일 때 채팅/모달 입력만 처리
static void mp_handle_opponent_turn(UIManager *ui_mgr, MultiplayerGame *game, InputHandler *input_handler)
{
    // 모달이 활성화되어 있으면 모달 입력만 처리
    if (modal_ui_is_active(&game->modal_ui))
    {
        InputEvent event = input_handler_get_event(input_handler, ui_mgr->board_win);
        int ch = event.key_code;
        if (ch != ERR)
        {
            ModalResult result = modal_ui_handle_input(&game->modal_ui, ch);

            // 모달 결과 처리
            switch (result)
            {
            case MODAL_RESULT_YES:
                // 기권 확정
                if (game->modal_ui.type == MODAL_GIVEUP)
                {
                    game->game_over = true;
                    game->result = (game->me.color == BLACK) ? GAME_WHITE_WIN : GAME_BLACK_WIN;
                    modal_ui_close(&game->modal_ui);

                    // 기권 메시지 전송
                    Message msg;
                    protocol_init_message(&msg, MSG_COMMAND, game->network.sequence_number++);
                    msg.payload.command.command_type = CMD_GIVEUP;
                    mp_send_with_error_check(game, &msg, "sending giveup");
                }
                break;

            case MODAL_RESULT_NO:
            case MODAL_RESULT_CANCEL:
                modal_ui_close(&game->modal_ui);
                break;

            case MODAL_RESULT_ACCEPT:
                // 무르기/Swap 수락
                if (game->modal_ui.type == MODAL_UNDO_RESPONSE)
                {
                    // 수락 응답 전송
                    Message msg;
                    protocol_init_message(&msg, MSG_COMMAND_RESPONSE, game->network.sequence_number++);
                    msg.payload.command_response.command_type = CMD_UNDO;
                    msg.payload.command_response.accepted = 1;
                    strncpy(msg.payload.command_response.message, "Undo accepted", sizeof(msg.payload.command_response.message) - 1);
                    network_send_message(&game->network, &msg);

                    // 로컬에서도 Undo 적용 - 마지막 두 수 되돌리기
                    // 턴 변경 없음 (두 수 되돌리면 턴이 그대로)
                    board_undo_last_move(&game->board);
                    board_undo_last_move(&game->board);

                    modal_ui_close(&game->modal_ui);
                    chat_ui_add_message(&game->chat_ui, "Undo accepted", CHAT_MSG_SYSTEM);
                    game->first_render = true;
                }
                else if (game->modal_ui.type == MODAL_SWAP_RESPONSE)
                {
                    // Swap 수락 응답 전송
                    Message msg;
                    protocol_init_message(&msg, MSG_COMMAND_RESPONSE, game->network.sequence_number++);
                    msg.payload.command_response.command_type = CMD_SWAP;
                    msg.payload.command_response.accepted = 1;
                    strncpy(msg.payload.command_response.message, "Swap accepted", sizeof(msg.payload.command_response.message) - 1);
                    network_send_message(&game->network, &msg);

                    // 로컬에서도 Swap 적용
                    Stone temp = game->me.color;
                    game->me.color = game->opponent.color;
                    game->opponent.color = temp;
                    game->swap_used = true;

                    modal_ui_close(&game->modal_ui);
                    chat_ui_add_message(&game->chat_ui, "Swap accepted", CHAT_MSG_SYSTEM);
                    game->first_render = true;
                }
                break;

            case MODAL_RESULT_DECLINE:
                // 무르기/Swap 거절
                if (game->modal_ui.type == MODAL_UNDO_RESPONSE)
                {
                    // 거절 응답 전송
                    Message msg;
                    protocol_init_message(&msg, MSG_COMMAND_RESPONSE, game->network.sequence_number++);
                    msg.payload.command_response.command_type = CMD_UNDO;
                    msg.payload.command_response.accepted = 0;
                    strncpy(msg.payload.command_response.message, "Undo declined", sizeof(msg.payload.command_response.message) - 1);
                    network_send_message(&game->network, &msg);

                    chat_ui_add_message(&game->chat_ui, "Undo declined", CHAT_MSG_SYSTEM);
                }
                else if (game->modal_ui.type == MODAL_SWAP_RESPONSE)
                {
                    // 거절 응답 전송
                    Message msg;
                    protocol_init_message(&msg, MSG_COMMAND_RESPONSE, game->network.sequence_number++);
                    msg.payload.command_response.command_type = CMD_SWAP;
                    msg.payload.command_response.accepted = 0;
                    strncpy(msg.payload.command_response.message, "Swap declined", sizeof(msg.payload.command_response.message) - 1);
                    network_send_message(&game->network, &msg);

                    chat_ui_add_message(&game->chat_ui, "Swap declined", CHAT_MSG_SYSTEM);
                }
                modal_ui_close(&game->modal_ui);
                break;

            case MODAL_RESULT_OK:
                modal_ui_close(&game->modal_ui);
                break;

            default:
                break;
            }

            // 모달이 닫혔으면 전체 재렌더링
            if (!modal_ui_is_active(&game->modal_ui))
            {
                game->first_render = true;
            }
        }
        return;
    }

    // 채팅 모드 확인
    if (chat_ui_is_input_mode(&game->chat_ui))
    {
        InputEvent event = input_handler_get_event(input_handler, ui_mgr->board_win);
        int ch = event.key_code;
        if (ch != ERR)
        {
            if (ch == '\n' || ch == KEY_ENTER)
            {
                // 메시지 전송
                const char *msg_text = chat_ui_get_message(&game->chat_ui);
                if (strlen(msg_text) > 0)
                {
                    // 명령어 체크
                    if (command_is_command(msg_text))
                    {
                        CommandResult cmd = command_parse(msg_text);
                        if (cmd.valid)
                        {
                            char modal_msg[MODAL_MAX_MESSAGE_LENGTH];

                            switch (cmd.type)
                            {
                            case CMD_GIVEUP:
                                snprintf(modal_msg, sizeof(modal_msg), "Are you sure you want to give up?");
                                modal_ui_show(&game->modal_ui, MODAL_GIVEUP, modal_msg);
                                break;

                            case CMD_UNDO:
                                // 본인 차례일 때만 Undo 요청 가능
                                chat_ui_add_message(&game->chat_ui, "Undo only on your turn", CHAT_MSG_SYSTEM);
                                break;

                            case CMD_SWAP:
                            {
                                if (game->me.color != WHITE)
                                {
                                    chat_ui_add_message(&game->chat_ui, "Only WHITE can use swap", CHAT_MSG_SYSTEM);
                                }
                                else if (game->swap_used)
                                {
                                    chat_ui_add_message(&game->chat_ui, "Swap already used", CHAT_MSG_SYSTEM);
                                }
                                else if (board_get_move_count(&game->board) < 3)
                                {
                                    chat_ui_add_message(&game->chat_ui, "Swap available after 3 moves", CHAT_MSG_SYSTEM);
                                }
                                else
                                {
                                    Message msg;
                                    protocol_init_message(&msg, MSG_COMMAND, game->network.sequence_number++);
                                    msg.payload.command.command_type = CMD_SWAP;
                                    network_send_message(&game->network, &msg);

                                    snprintf(modal_msg, sizeof(modal_msg), "Waiting for opponent's response...");
                                    modal_ui_show(&game->modal_ui, MODAL_SWAP_REQUEST, modal_msg);
                                    chat_ui_add_message(&game->chat_ui, "Swap request sent", CHAT_MSG_SYSTEM);
                                }
                            }
                            break;

                            case CMD_QUIT:
                            {
                                Message msg;
                                protocol_init_message(&msg, MSG_COMMAND, game->network.sequence_number++);
                                msg.payload.command.command_type = CMD_QUIT;
                                network_send_message(&game->network, &msg);
                                game->game_over = true;
                                game->quit_requested = true; // 메인 화면으로 바로 이동
                            }
                            break;

                            case CMD_HELP:
                                // 도움말 출력
                                chat_ui_add_message(&game->chat_ui, "=== Commands ===", CHAT_MSG_SYSTEM);
                                chat_ui_add_message(&game->chat_ui, "/help  - Show help", CHAT_MSG_SYSTEM);
                                chat_ui_add_message(&game->chat_ui, "/quit  - Leave game", CHAT_MSG_SYSTEM);
                                chat_ui_add_message(&game->chat_ui, "/undo  - Request undo", CHAT_MSG_SYSTEM);
                                chat_ui_add_message(&game->chat_ui, "/giveup - Forfeit", CHAT_MSG_SYSTEM);
                                chat_ui_add_message(&game->chat_ui, "/swap  - Swap colors", CHAT_MSG_SYSTEM);
                                break;

                            default:
                                break;
                            }
                        }
                        else
                        {
                            chat_ui_add_message(&game->chat_ui, cmd.error_message, CHAT_MSG_SYSTEM);
                        }
                    }
                    else
                    {
                        // 일반 채팅 전송
                        Message msg;
                        protocol_init_message(&msg, MSG_CHAT, game->network.sequence_number++);
                        strncpy(msg.payload.chat.message, msg_text, sizeof(msg.payload.chat.message) - 1);
                        network_send_message(&game->network, &msg);

                        mp_broadcast_chat_to_spectators(game, &msg);

                        chat_ui_add_message(&game->chat_ui, msg_text, CHAT_MSG_USER);
                    }
                }
                chat_ui_exit_input_mode(&game->chat_ui);
            }
            else
            {
                chat_ui_handle_input(&game->chat_ui, ch);
            }
        }
        return;
    }

    // 채팅 모드가 아니면 Enter 또는 T 키로 채팅 모드 진입 가능
    InputEvent event = input_handler_get_event(input_handler, ui_mgr->board_win);

    if (event.key_code == '\n' || event.key_code == 't' || event.key_code == 'T')
    {
        chat_ui_enter_input_mode(&game->chat_ui);
    }
}

// 관전자 관련 함수들

// 관전자에게 현재 게임 상태 전송
static void mp_send_game_state_to_spectator(MultiplayerGame *game, int spectator_index)
{
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
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            msg.payload.game_state.board_state[row * BOARD_SIZE + col] = game->board.cells[row][col];
        }
    }

    network_send_to_spectator(&game->network, spectator_index, &msg);
}

// 새로운 관전자 연결 처리
static void mp_handle_spectator_connections(MultiplayerGame *game)
{
    // 관전자 연결 시도 체크
    if (network_server_accept_spectator(&game->network))
    {
        // 새로운 관전자 연결됨
        int new_spectator_index = game->network.spectator_count - 1;

        // 관전자로부터 MSG_SPECTATOR_CONNECT 대기
        uint8_t temp_buffer[1024];
        int spectator_fd = game->network.spectator_fds[new_spectator_index];

        // 논블로킹 recv 시도
        ssize_t received = recv(spectator_fd, temp_buffer, sizeof(temp_buffer), MSG_DONTWAIT);
        if (received > 0)
        {
            Message connect_msg;
            int result = protocol_deserialize(&connect_msg, temp_buffer, received);

            if (result > 0 && connect_msg.header.type == MSG_SPECTATOR_CONNECT)
            {
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
                for (int i = 0; i < MAX_SPECTATORS; i++)
                {
                    if (i != new_spectator_index && game->network.spectator_fds[i] >= 0)
                    {
                        network_send_to_spectator(&game->network, i, &join_msg);
                    }
                }
            }
        }
    }
}

// 모든 관전자에게 메시지 브로드캐스트
static void mp_broadcast_move_to_spectators(MultiplayerGame *game, const Message *move_msg)
{
    network_broadcast_to_spectators(&game->network, move_msg);
}

static void mp_broadcast_chat_to_spectators(MultiplayerGame *game, const Message *chat_msg)
{
    network_broadcast_to_spectators(&game->network, chat_msg);
}

// 에러 체크를 포함한 네트워크 메시지 전송
static bool mp_send_with_error_check(MultiplayerGame *game, const Message *msg, const char *error_context)
{
    int result = network_send_message(&game->network, msg);

    if (result < 0)
    {
        // Send 실패 - 연결 상태 체크
        if (!network_is_connected(&game->network) ||
            game->network.state == NET_DISCONNECTED ||
            game->network.state == NET_ERROR)
        {

            if (!game->game_over)
            { // 아직 게임이 끝나지 않았으면
                char modal_msg[MODAL_MAX_MESSAGE_LENGTH];
                snprintf(modal_msg, sizeof(modal_msg),
                         "Connection lost while %s. Game will end.",
                         error_context ? error_context : "sending data");
                modal_ui_show(&game->modal_ui, MODAL_ERROR, modal_msg);

                char log_msg[128];
                snprintf(log_msg, sizeof(log_msg), "Connection lost!");
                log_ui_add_message(&game->log_ui, log_msg);
                chat_ui_add_message(&game->chat_ui, "Connection lost", CHAT_MSG_SYSTEM);

                game->game_over = true;
            }
        }
        return false;
    }

    return true;
}
