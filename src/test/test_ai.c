#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../game/core/board.h"
#include "../game/ai/ai_engine.h"
#include "../game/core/game_logic.h"

void test_ai_initialization()
{
    printf("Testing AI initialization...\n");

    AIEngine ai_easy, ai_hard;
    ai_init(&ai_easy, AI_EASY, WHITE);
    ai_init(&ai_hard, AI_HARD, BLACK);

    assert(ai_easy.difficulty == AI_EASY);
    assert(ai_easy.ai_stone == WHITE);
    assert(ai_hard.difficulty == AI_HARD);
    assert(ai_hard.ai_stone == BLACK);

    printf("✓ AI initialization test passed\n");
}

void test_ai_first_move()
{
    printf("Testing AI first move (should be center)...\n");

    Board board;
    board_init(&board);

    AIEngine ai;
    ai_init(&ai, AI_EASY, BLACK);

    Position move;
    bool result = ai_get_next_move(&ai, &board, &move);

    assert(result == true);
    assert(move.row == BOARD_SIZE / 2);
    assert(move.col == BOARD_SIZE / 2);

    printf("✓ AI first move test passed (center: %d, %d)\n", move.row, move.col);
}

void test_ai_winning_move()
{
    printf("Testing AI winning move detection...\n");

    Board board;
    board_init(&board);

    // AI가 WHITE라고 가정
    // WHITE가 4개를 연속으로 놓고, 5번째를 놓아서 승리할 수 있는 상황
    // 가로로 4개 놓기: (9, 5), (9, 6), (9, 7), (9, 8)
    board_place_stone(&board, 9, 5, WHITE);
    board_place_stone(&board, 9, 6, WHITE);
    board_place_stone(&board, 9, 7, WHITE);
    board_place_stone(&board, 9, 8, WHITE);

    AIEngine ai;
    ai_init(&ai, AI_EASY, WHITE);

    Position move;
    bool result = ai_get_next_move(&ai, &board, &move);

    assert(result == true);
    // AI는 (9, 4) 또는 (9, 9)에 놓아서 5개를 완성해야 함
    assert((move.row == 9 && move.col == 4) || (move.row == 9 && move.col == 9));

    printf("✓ AI winning move test passed (move: %d, %d)\n", move.row, move.col);
}

void test_ai_blocking_move()
{
    printf("Testing AI blocking opponent's winning move...\n");

    Board board;
    board_init(&board);

    // AI는 WHITE
    // BLACK이 4개를 연속으로 놓은 상황
    board_place_stone(&board, 5, 5, BLACK);
    board_place_stone(&board, 5, 6, BLACK);
    board_place_stone(&board, 5, 7, BLACK);
    board_place_stone(&board, 5, 8, BLACK);

    AIEngine ai;
    ai_init(&ai, AI_EASY, WHITE);

    Position move;
    bool result = ai_get_next_move(&ai, &board, &move);

    assert(result == true);
    // AI는 (5, 4) 또는 (5, 9)에 놓아서 BLACK의 승리를 막아야 함
    assert((move.row == 5 && move.col == 4) || (move.row == 5 && move.col == 9));

    printf("✓ AI blocking move test passed (block at: %d, %d)\n", move.row, move.col);
}

void test_ai_hard_vs_easy()
{
    printf("Testing AI Hard vs AI Easy...\n");

    Board board;
    board_init(&board);

    AIEngine ai_easy, ai_hard;
    ai_init(&ai_easy, AI_EASY, BLACK);
    ai_init(&ai_hard, AI_HARD, WHITE);

    // 간단한 게임 시뮬레이션 (최대 20수)
    Stone current_player = BLACK;
    int move_count = 0;
    const int MAX_MOVES = 20;

    while (move_count < MAX_MOVES)
    {
        Position move;
        bool result;

        if (current_player == BLACK)
        {
            result = ai_get_next_move(&ai_easy, &board, &move);
        }
        else
        {
            result = ai_get_next_move(&ai_hard, &board, &move);
        }

        assert(result == true);
        assert(board_is_valid_position(move.row, move.col));
        assert(board_is_empty(&board, move.row, move.col));

        board_place_stone(&board, move.row, move.col, current_player);

        // 승리 체크
        GameResult game_result = game_check_winner(&board);
        if (game_result != GAME_ONGOING)
        {
            printf("  Game ended after %d moves: ", move_count + 1);
            if (game_result == GAME_BLACK_WIN)
            {
                printf("BLACK (Easy) wins\n");
            }
            else if (game_result == GAME_WHITE_WIN)
            {
                printf("WHITE (Hard) wins\n");
            }
            else
            {
                printf("Draw\n");
            }
            break;
        }

        current_player = (current_player == BLACK) ? WHITE : BLACK;
        move_count++;
    }

    printf("✓ AI vs AI test passed (%d moves played)\n", move_count);
}

int main()
{
    printf("=== AI Engine Tests ===\n\n");

    test_ai_initialization();
    test_ai_first_move();
    test_ai_winning_move();
    test_ai_blocking_move();
    test_ai_hard_vs_easy();

    printf("\n=== All AI tests passed! ===\n");
    return 0;
}
