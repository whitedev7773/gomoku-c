#include "multiplayer.h"
#include "mp_common.h"
#include <unistd.h>
#include <string.h>
#include <time.h>

// ============================================================================
// 호스트 모드 실행
// ============================================================================

int multiplayer_run_host(int port, GameRule rule)
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

    printf("=== GOMOKU MULTIPLAYER - HOST ===\n");
    printf("Server started on %s:%d\n", game.network.local_ip, port);
    printf("Waiting for client to connect...\n");
    printf("(Press Ctrl+C to cancel)\n\n");

    network_set_nonblocking(game.network.socket_fd, true);

    while (!network_server_accept_client(&game.network))
    {
        usleep(100000);
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
    snprintf(start_msg, sizeof(start_msg), "Game started! You: %s (BLACK), Opponent: %s (WHITE)",
             game.me.name, game.opponent.name);
    log_ui_add_message(&game.log_ui, start_msg);
    chat_ui_add_message(&game.chat_ui, start_msg, CHAT_MSG_SYSTEM);

    InputHandler input_handler;
    input_handler_init(&input_handler);

    // 게임 시작 메시지 전송
    protocol_init_message(&msg, MSG_GAME_START, game.network.sequence_number++);
    msg.payload.game_start.your_turn = 0;
    network_send_message(&game.network, &msg);

    bool game_running = true;

    while (game_running)
    {
        mp_handle_spectator_connections(&game);
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

int multiplayer_run_client(const char *server_ip, int port, GameRule rule)
{
    MultiplayerGame game;
    memset(&game, 0, sizeof(game));
    GameRule received_rule = rule;

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

    printf("Enter your name (max 8 chars): ");
    fgets(game.me.name, sizeof(game.me.name), stdin);
    game.me.name[strcspn(game.me.name, "\n")] = '\0';
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

    // UI 및 게임 컴포넌트 초기화
    UIManager ui_mgr;
    if (!mp_init_game_ui(&ui_mgr, &game, received_rule))
    {
        network_cleanup(&game.network);
        return -1;
    }

    char start_msg[128];
    snprintf(start_msg, sizeof(start_msg), "Game started! You: %s (%s), Opponent: %s (%s)",
             game.me.name, game.me.color == BLACK ? "BLACK" : "WHITE",
             game.opponent.name, game.opponent.color == BLACK ? "BLACK" : "WHITE");
    log_ui_add_message(&game.log_ui, start_msg);
    chat_ui_add_message(&game.chat_ui, start_msg, CHAT_MSG_SYSTEM);

    InputHandler input_handler;
    input_handler_init(&input_handler);

    bool game_running = true;

    while (game_running)
    {
        mp_handle_network_messages(&ui_mgr, &game);
        mp_render_game(&ui_mgr, &game);

        mp_check_game_end(&game, false); // is_host = false

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
