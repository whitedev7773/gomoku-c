#include <stdio.h>
#include <assert.h>
#include "../game/core/board.h"
#include "../game/core/game_logic.h"
#include "../game/core/turn_manager.h"
#include "../game/feature/game_logger.h"
#include "../game/core/rules.h"

void test_board()
{
    printf("Testing Board...\n");

    Board board;
    board_init(&board);

    assert(board.move_count == 0);
    assert(board_is_empty(&board, 9, 9));

    assert(board_place_stone(&board, 9, 9, BLACK));
    assert(!board_is_empty(&board, 9, 9));
    assert(board_get_stone(&board, 9, 9) == BLACK);
    assert(board.move_count == 1);

    assert(!board_place_stone(&board, 9, 9, WHITE));

    printf("Board tests PASSED!\n\n");
}

void test_game_logic()
{
    printf("Testing Game Logic...\n");

    Board board;
    board_init(&board);

    board_place_stone(&board, 9, 9, BLACK);
    board_place_stone(&board, 9, 10, BLACK);
    board_place_stone(&board, 9, 11, BLACK);
    board_place_stone(&board, 9, 12, BLACK);
    board_place_stone(&board, 9, 13, BLACK);

    assert(game_has_five_in_row(&board, 9, 9, BLACK, NULL));
    assert(game_check_winner(&board) == GAME_BLACK_WIN);

    printf("Game Logic tests PASSED!\n\n");
}

void test_turn_manager()
{
    printf("Testing Turn Manager...\n");

    TurnManager manager;
    turn_manager_init(&manager, BLACK);

    assert(turn_manager_get_current_player(&manager) == BLACK);

    turn_manager_next_turn(&manager);
    assert(turn_manager_get_current_player(&manager) == WHITE);

    turn_manager_next_turn(&manager);
    assert(turn_manager_get_current_player(&manager) == BLACK);

    printf("Turn Manager tests PASSED!\n\n");
}

void test_game_logger()
{
    printf("Testing Game Logger...\n");

    GameLogger logger;
    assert(logger_init(&logger));

    assert(logger_log_move(&logger, BLACK, 9, 9, 1));
    assert(logger_log_move(&logger, WHITE, 10, 10, 2));
    assert(logger_log_event(&logger, "Test event"));

    logger_close(&logger);

    printf("Game Logger tests PASSED!\n\n");
}

void test_renju_rules()
{
    printf("Testing Renju Rules...\n");

    Board board;
    board_init(&board);

    board_place_stone(&board, 9, 9, BLACK);
    board_place_stone(&board, 9, 10, BLACK);
    board_place_stone(&board, 9, 11, BLACK);
    board_place_stone(&board, 9, 12, BLACK);
    board_place_stone(&board, 9, 13, BLACK);
    board_place_stone(&board, 9, 14, BLACK);

    assert(rules_check_overline(&board, 9, 8, BLACK));

    printf("Renju Rules tests PASSED!\n\n");
}

void test_swap_rules()
{
    printf("Testing Swap Rules...\n");

    Board board;
    board_init(&board);

    board_place_stone(&board, 9, 9, BLACK);
    assert(!rules_is_swap_available(&board));

    board_place_stone(&board, 10, 10, WHITE);
    assert(!rules_is_swap_available(&board));

    board_place_stone(&board, 11, 11, BLACK);
    assert(rules_is_swap_available(&board));

    rules_apply_swap(&board);
    assert(board_get_stone(&board, 9, 9) == WHITE);
    assert(board_get_stone(&board, 10, 10) == BLACK);

    printf("Swap Rules tests PASSED!\n\n");
}

int main()
{
    printf("=== Phase 2 Tests ===\n\n");

    test_board();
    test_game_logic();
    test_turn_manager();
    test_game_logger();
    test_renju_rules();
    test_swap_rules();

    printf("=== All Phase 2 Tests PASSED! ===\n");

    return 0;
}
