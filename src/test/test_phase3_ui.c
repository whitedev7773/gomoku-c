#include <stdio.h>
#include <unistd.h>
#include "../ui/ui_manager.h"
#include "../ui/board_ui.h"
#include "../ui/input_handler.h"
#include "../ui/game_info_ui.h"
#include "../ui/log_ui.h"
#include "../game/board.h"
#include "../game/game_logic.h"
#include "../game/turn_manager.h"

int main() {
    UIManager manager;
    Board board;
    TurnManager turn_mgr;
    BoardCursor cursor;
    GameInfoUI game_info;
    LogUI log_ui;

    // Initialize
    if (!ui_manager_init(&manager)) {
        fprintf(stderr, "Failed to initialize UI. Terminal too small?\n");
        return 1;
    }

    board_init(&board);
    board_ui_init_cursor(&cursor);
    turn_manager_init(&turn_mgr, BLACK);
    game_info_ui_init(&game_info);
    log_ui_init(&log_ui);

    log_ui_add_message(&log_ui, "Game started!");
    log_ui_add_message(&log_ui, "Use arrow keys to move, SPACE to place stone");
    log_ui_add_message(&log_ui, "Press Q to quit");

    bool running = true;
    bool game_over = false;

    while (running) {
        // Render UI
        board_ui_render(manager.board_win, &board, &cursor);
        game_info_ui_render_bottom(manager.bottom_win, &board, &turn_mgr, &game_info);
        log_ui_render(manager.bottom_win, &log_ui, 1, 42);

        // Draw info window placeholder
        wclear(manager.info_win);
        box(manager.info_win, 0, 0);
        mvwprintw(manager.info_win, 0, 2, " P1: Player1 ");
        wrefresh(manager.info_win);

        // Draw chat window placeholder
        wclear(manager.chat_win);
        box(manager.chat_win, 0, 0);
        mvwprintw(manager.chat_win, 1, 2, "(Chat UI - Phase 6)");
        wrefresh(manager.chat_win);

        // Draw chat input placeholder
        wclear(manager.chat_input_win);
        box(manager.chat_input_win, 0, 0);
        mvwprintw(manager.chat_input_win, 1, 2, "Enter to Chat...");
        wrefresh(manager.chat_input_win);

        // Handle input
        InputEvent event = input_get_event(manager.board_win);

        if (game_over) {
            if (event.action == INPUT_QUIT) {
                running = false;
            }
            continue;
        }

        switch (event.action) {
            case INPUT_MOVE_UP:
                board_ui_move_cursor(&cursor, -1, 0);
                break;
            case INPUT_MOVE_DOWN:
                board_ui_move_cursor(&cursor, 1, 0);
                break;
            case INPUT_MOVE_LEFT:
                board_ui_move_cursor(&cursor, 0, -1);
                break;
            case INPUT_MOVE_RIGHT:
                board_ui_move_cursor(&cursor, 0, 1);
                break;
            case INPUT_PLACE_STONE:
                if (board_place_stone(&board, cursor.cursor_row, cursor.cursor_col,
                                     turn_manager_get_current_player(&turn_mgr))) {
                    char msg[64];
                    snprintf(msg, sizeof(msg), "%s placed stone at %c%d",
                            turn_manager_get_current_player(&turn_mgr) == BLACK ? "BLACK" : "WHITE",
                            board_ui_col_to_char(cursor.cursor_col),
                            cursor.cursor_row + 1);
                    log_ui_add_message(&log_ui, msg);

                    // Check win
                    GameResult result = game_check_winner(&board);
                    if (result != GAME_ONGOING) {
                        game_over = true;
                        if (result == GAME_BLACK_WIN) {
                            log_ui_add_message(&log_ui, "BLACK WINS!");
                        } else if (result == GAME_WHITE_WIN) {
                            log_ui_add_message(&log_ui, "WHITE WINS!");
                        } else {
                            log_ui_add_message(&log_ui, "DRAW!");
                        }
                    } else {
                        turn_manager_next_turn(&turn_mgr);
                    }
                }
                break;
            case INPUT_QUIT:
                running = false;
                break;
            default:
                break;
        }
    }

    ui_manager_cleanup(&manager);

    printf("Game ended!\n");
    return 0;
}
