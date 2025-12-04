#include "spectator.h"
#include "../ui/ui_manager.h"
#include "../ui/board_ui.h"
#include "../ui/game_info_ui.h"
#include "../ui/chat_ui.h"
#include "../ui/modal_ui.h"
#include "../ui/input_handler.h"
#include "../ui/ingame_border.h"
#include "game_logic.h"
#include "turn_manager.h"
#include "../network/protocol.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>

// 관전자 게임 상태
typedef struct
{
    Board board;
    TurnManager turn_mgr;
    ChatUI chat_ui;
    ModalUI modal_ui;
    NetworkManager network;

    char player1_name[MAX_PLAYER_NAME];
    char player2_name[MAX_PLAYER_NAME];
    int spectator_count;
    int max_spectators;

    // 현재 턴 플레이어의 커서 (관전자용)
    BoardCursor current_player_cursor;

    time_t game_start_time;
    int turn_remaining_time; // 턴 남은 시간
    bool game_active;
    bool need_render; // 렌더링 필요 여부
    bool game_ended;  // 게임 종료 여부
} SpectatorGame;

// 초기화
static void spectator_init(SpectatorGame *game, const char *spectator_name)
{
    memset(game, 0, sizeof(SpectatorGame));

    board_init(&game->board);
    turn_manager_init(&game->turn_mgr, BLACK); // 기본값으로 BLACK, 나중에 서버 상태로 갱신
    chat_ui_init(&game->chat_ui);
    modal_ui_init(&game->modal_ui);
    network_init_spectator(&game->network);

    game->max_spectators = MAX_SPECTATORS;
    game->spectator_count = 0;
    game->game_start_time = time(NULL);
    game->turn_remaining_time = 20;
    game->game_active = false;
    game->need_render = true;
    game->game_ended = false;

    // 커서 초기화
    game->current_player_cursor.cursor_row = BOARD_SIZE / 2;
    game->current_player_cursor.cursor_col = BOARD_SIZE / 2;
    game->current_player_cursor.prev_cursor_row = BOARD_SIZE / 2;
    game->current_player_cursor.prev_cursor_col = BOARD_SIZE / 2;
}

// 네트워크 메시지 처리
static bool spectator_handle_network_messages(SpectatorGame *game)
{
    Message msg;
    int result = network_receive_message(&game->network, &msg);

    if (result < 0)
    {
        // 연결 끊김
        return false;
    }

    if (result == 0)
    {
        // 데이터 없음
        return true;
    }

    switch (msg.header.type)
    {
    case MSG_SPECTATOR_CONNECT_ACK:
        if (msg.payload.spectator_connect_ack.accepted)
        {
            game->spectator_count = msg.payload.spectator_connect_ack.spectator_count;
            game->max_spectators = msg.payload.spectator_connect_ack.max_spectators;
        }
        else
        {
            // 연결 거부
            if (msg.payload.spectator_connect_ack.error_code == ERR_NO_GAME)
            {
                chat_ui_add_message(&game->chat_ui, "No game in progress", CHAT_MSG_SYSTEM);
            }
            else if (msg.payload.spectator_connect_ack.error_code == ERR_SPECTATOR_FULL)
            {
                chat_ui_add_message(&game->chat_ui, "Spectator slots full", CHAT_MSG_SYSTEM);
            }
            return false;
        }
        break;

    case MSG_GAME_STATE:
        // 전체 게임 상태 로드
        strncpy(game->player1_name, msg.payload.game_state.player1_name, MAX_PLAYER_NAME);
        strncpy(game->player2_name, msg.payload.game_state.player2_name, MAX_PLAYER_NAME);

        // 보드 상태 복원
        for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++)
        {
            int row = i / BOARD_SIZE;
            int col = i % BOARD_SIZE;
            game->board.cells[row][col] = msg.payload.game_state.board_state[i];
        }

        // 턴 정보 설정
        game->turn_mgr.current_player = msg.payload.game_state.current_turn;
        game->game_active = true;

        chat_ui_add_message(&game->chat_ui, "Game loaded", CHAT_MSG_SYSTEM);
        break;

    case MSG_MOVE:
        // 상대방 수 적용
        if (game->game_active)
        {
            if (board_place_stone(&game->board, msg.payload.move.row, msg.payload.move.col,
                                  msg.payload.move.stone))
            {
                turn_manager_next_turn(&game->turn_mgr);
                game->turn_remaining_time = 20; // 타이머 리셋
                game->need_render = true;
            }
        }
        break;

    case MSG_CHAT:
        // 채팅 메시지 수신
        chat_ui_add_message(&game->chat_ui, msg.payload.chat.message, CHAT_MSG_OPPONENT);
        game->need_render = true;
        break;

    case MSG_SPECTATOR_JOIN:
        // 다른 관전자 입장
        game->spectator_count = msg.payload.spectator_join_leave.spectator_count;
        {
            char sys_msg[128];
            snprintf(sys_msg, sizeof(sys_msg), "%s joined",
                     msg.payload.spectator_join_leave.spectator_name);
            chat_ui_add_message(&game->chat_ui, sys_msg, CHAT_MSG_SYSTEM);
        }
        game->need_render = true;
        break;

    case MSG_SPECTATOR_LEAVE:
        // 다른 관전자 퇴장
        game->spectator_count = msg.payload.spectator_join_leave.spectator_count;
        {
            char sys_msg[128];
            snprintf(sys_msg, sizeof(sys_msg), "%s left",
                     msg.payload.spectator_join_leave.spectator_name);
            chat_ui_add_message(&game->chat_ui, sys_msg, CHAT_MSG_SYSTEM);
        }
        game->need_render = true;
        break;

    case MSG_DISCONNECT:
        // 게임 종료
        chat_ui_add_message(&game->chat_ui, "Game ended", CHAT_MSG_SYSTEM);
        game->game_active = false;
        game->need_render = true;
        break;

    case MSG_PING:
        // 서버로부터 PING - PONG 응답 (타이머 동기화는 별도 메시지로)
        game->need_render = true;
        break;

    case MSG_CURSOR_UPDATE:
        // 현재 턴 플레이어의 커서 위치 업데이트
        game->current_player_cursor.prev_cursor_row = game->current_player_cursor.cursor_row;
        game->current_player_cursor.prev_cursor_col = game->current_player_cursor.cursor_col;
        game->current_player_cursor.cursor_row = msg.payload.cursor.row;
        game->current_player_cursor.cursor_col = msg.payload.cursor.col;
        game->need_render = true;
        break;

    case MSG_GAME_RESULT:
        // 게임 결과 수신 - 모달 표시
        {
            game->game_ended = true;
            game->game_active = false;
            modal_ui_show(&game->modal_ui, MODAL_GAME_RESULT, msg.payload.game_result.message);
            chat_ui_add_message(&game->chat_ui, msg.payload.game_result.message, CHAT_MSG_SYSTEM);
        }
        game->need_render = true;
        break;

    default:
        break;
    }

    return true;
}

// UI 렌더링
static void spectator_render_game(UIManager *ui_mgr, const SpectatorGame *game)
{
    // 보드 렌더링 (현재 턴 플레이어의 커서 표시)
    board_ui_render(ui_mgr->board_win, &game->board, &game->current_player_cursor);

    // 게임 정보 렌더링
    int max_y, max_x;
    getmaxyx(ui_mgr->info_win, max_y, max_x);

    werase(ui_mgr->info_win);
    box(ui_mgr->info_win, 0, 0);
    mvwprintw(ui_mgr->info_win, 0, 2, " SPECTATOR MODE ");

    // 플레이어 정보
    mvwprintw(ui_mgr->info_win, 2, 2, "P1 (BLACK): %s", game->player1_name);
    mvwprintw(ui_mgr->info_win, 3, 2, "P2 (WHITE): %s", game->player2_name);

    // 현재 턴 및 타이머
    Stone current_turn_stone = turn_manager_get_current_player(&game->turn_mgr);
    const char *current_player = (current_turn_stone == BLACK) ? game->player1_name : game->player2_name;
    mvwprintw(ui_mgr->info_win, 5, 2, "Turn: %s [%02ds]", current_player, game->turn_remaining_time);

    // 타이머 프로그레스 바
    int filled = (game->turn_remaining_time * 15) / 20;
    mvwprintw(ui_mgr->info_win, 6, 2, "[");
    for (int i = 0; i < 15; i++)
    {
        if (i < filled)
            wprintw(ui_mgr->info_win, "█");
        else
            wprintw(ui_mgr->info_win, "░");
    }
    wprintw(ui_mgr->info_win, "]");

    // 관전자 수
    mvwprintw(ui_mgr->info_win, 8, 2, "Spectators: %d/%d",
              game->spectator_count, game->max_spectators);

    // 게임 업타임
    time_t current_time = time(NULL);
    int uptime_sec = (int)difftime(current_time, game->game_start_time);
    int minutes = uptime_sec / 60;
    int seconds = uptime_sec % 60;
    mvwprintw(ui_mgr->info_win, 9, 2, "Play Time: %02d:%02d", minutes, seconds);

    // 조작 안내
    mvwprintw(ui_mgr->info_win, max_y - 2, 2, "Press 'q' to quit");

    wrefresh(ui_mgr->info_win);

    // 채팅 UI 렌더링
    chat_ui_render(ui_mgr->chat_win, &game->chat_ui);
    wrefresh(ui_mgr->chat_win);

    // 모달이 활성화되어 있으면 가장 위에 렌더링
    if (modal_ui_is_active(&game->modal_ui))
    {
        modal_ui_render(stdscr, &game->modal_ui);
    }
}

int spectator_run(const char *server_ip, int port, const char *spectator_name)
{
    SpectatorGame game;
    spectator_init(&game, spectator_name);

    // UI 초기화
    UIManager ui_mgr;
    ui_manager_init(&ui_mgr);

    // 인게임 Border 그리기
    ingame_border_draw();
    refresh();

    // 게임패드 입력 핸들러 초기화
    InputHandler input_handler;
    input_handler_init(&input_handler);

    // 서버에 연결
    chat_ui_add_message(&game.chat_ui, "Connecting to server...", CHAT_MSG_SYSTEM);
    spectator_render_game(&ui_mgr, &game);

    if (!network_spectator_connect(&game.network, server_ip, port))
    {
        chat_ui_add_message(&game.chat_ui, "Failed to connect", CHAT_MSG_SYSTEM);
        spectator_render_game(&ui_mgr, &game);
        usleep(2000000); // 2초 대기
        input_handler_cleanup(&input_handler);
        ui_manager_cleanup(&ui_mgr);
        network_cleanup(&game.network);
        return 1;
    }

    // 관전자 연결 요청
    Message connect_msg;
    protocol_init_message(&connect_msg, MSG_SPECTATOR_CONNECT, game.network.sequence_number++);
    strncpy(connect_msg.payload.spectator_connect.spectator_name, spectator_name, MAX_PLAYER_NAME - 1);
    network_send_message(&game.network, &connect_msg);

    chat_ui_add_message(&game.chat_ui, "Waiting for response...", CHAT_MSG_SYSTEM);
    spectator_render_game(&ui_mgr, &game);

    // 승인 대기
    bool connected = false;
    time_t connect_start = time(NULL);
    while (difftime(time(NULL), connect_start) < 5.0)
    {
        Message ack_msg;
        int result = network_receive_message(&game.network, &ack_msg);

        if (result > 0 && ack_msg.header.type == MSG_SPECTATOR_CONNECT_ACK)
        {
            if (ack_msg.payload.spectator_connect_ack.accepted)
            {
                connected = true;
                game.spectator_count = ack_msg.payload.spectator_connect_ack.spectator_count;
                chat_ui_add_message(&game.chat_ui, "Connected!", CHAT_MSG_SYSTEM);
                spectator_render_game(&ui_mgr, &game);
            }
            else
            {
                if (ack_msg.payload.spectator_connect_ack.error_code == ERR_NO_GAME)
                {
                    chat_ui_add_message(&game.chat_ui, "No game in progress", CHAT_MSG_SYSTEM);
                }
                else if (ack_msg.payload.spectator_connect_ack.error_code == ERR_SPECTATOR_FULL)
                {
                    chat_ui_add_message(&game.chat_ui, "Spectator slots full", CHAT_MSG_SYSTEM);
                }
                spectator_render_game(&ui_mgr, &game);
                usleep(2000000);
            }
            break;
        }

        usleep(100000); // 100ms 대기
    }

    if (!connected)
    {
        chat_ui_add_message(&game.chat_ui, "Connection timeout", CHAT_MSG_SYSTEM);
        spectator_render_game(&ui_mgr, &game);
        usleep(2000000);
        input_handler_cleanup(&input_handler);
        ui_manager_cleanup(&ui_mgr);
        network_cleanup(&game.network);
        return 1;
    }

    // 게임 상태 대기
    bool state_received = false;
    time_t state_start = time(NULL);
    while (difftime(time(NULL), state_start) < 5.0)
    {
        Message state_msg;
        int result = network_receive_message(&game.network, &state_msg);

        if (result > 0 && state_msg.header.type == MSG_GAME_STATE)
        {
            // 게임 상태 로드
            strncpy(game.player1_name, state_msg.payload.game_state.player1_name, MAX_PLAYER_NAME);
            strncpy(game.player2_name, state_msg.payload.game_state.player2_name, MAX_PLAYER_NAME);

            for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++)
            {
                int row = i / BOARD_SIZE;
                int col = i % BOARD_SIZE;
                game.board.cells[row][col] = state_msg.payload.game_state.board_state[i];
            }

            game.turn_mgr.current_player = state_msg.payload.game_state.current_turn;
            game.game_active = true;
            state_received = true;

            chat_ui_add_message(&game.chat_ui, "Game loaded", CHAT_MSG_SYSTEM);
            spectator_render_game(&ui_mgr, &game);
            break;
        }

        usleep(100000);
    }

    if (!state_received)
    {
        chat_ui_add_message(&game.chat_ui, "Failed to load game state", CHAT_MSG_SYSTEM);
        spectator_render_game(&ui_mgr, &game);
        usleep(2000000);
        input_handler_cleanup(&input_handler);
        ui_manager_cleanup(&ui_mgr);
        network_cleanup(&game.network);
        return 1;
    }

    // 메인 루프 - non-blocking으로 실시간 동기화
    nodelay(ui_mgr.board_win, TRUE); // non-blocking 입력 모드

    time_t last_render_time = 0;
    time_t last_timer_update = time(NULL);

    while (true)
    {
        // 네트워크 메시지 처리 (여러 개 한번에)
        for (int i = 0; i < 10; i++)
        {
            if (!spectator_handle_network_messages(&game))
            {
                goto cleanup;
            }
        }

        // 1초마다 타이머 감소
        time_t current_time = time(NULL);
        if (current_time != last_timer_update)
        {
            if (game.game_active && game.turn_remaining_time > 0)
            {
                game.turn_remaining_time--;
            }
            game.need_render = true;
            last_timer_update = current_time;
        }

        // 렌더링 (필요할 때만 또는 1초마다)
        if (game.need_render || current_time != last_render_time)
        {
            spectator_render_game(&ui_mgr, &game);
            game.need_render = false;
            last_render_time = current_time;
        }

        // non-blocking 입력 처리
        int ch = wgetch(ui_mgr.board_win);
        if (ch == 'q' || ch == 'Q')
        {
            break;
        }

        usleep(10000); // 10ms 대기 (CPU 사용률 줄이기)
    }

cleanup:
    // 정리
    nodelay(ui_mgr.board_win, FALSE); // blocking 모드로 복원
    input_handler_cleanup(&input_handler);
    ui_manager_cleanup(&ui_mgr);
    network_cleanup(&game.network);

    return 0;
}
