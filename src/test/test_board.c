#include "test_framework.h"
#include "../game/core/board.h"
#include <stdio.h>

// 보드 초기화 테스트
void test_board_init()
{
    TEST_CASE("Board Initialization");

    Board board;
    board_init(&board);

    TEST_ASSERT_EQ(board.move_count, 0, "Initial move count should be 0");
    TEST_ASSERT_EQ(board.last_row, -1, "Initial last row should be -1");
    TEST_ASSERT_EQ(board.last_col, -1, "Initial last col should be -1");

    // 모든 칸이 비어있는지 확인
    bool all_empty = true;
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            if (board.cells[row][col] != EMPTY)
            {
                all_empty = false;
                break;
            }
        }
    }
    TEST_ASSERT_TRUE(all_empty, "All cells should be empty initially");
}

// Renju Rule 초기화 테스트
void test_board_init_with_rule()
{
    TEST_CASE("Board Initialization with Renju Rule");

    Board board;
    board_init_with_rule(&board, RULE_RENJU);

    TEST_ASSERT_EQ(board.rule, RULE_RENJU, "Rule should be set to RULE_RENJU");
    TEST_ASSERT_EQ(board.move_count, 0, "Move count should be 0");
}

// 돌 놓기 테스트
void test_board_place_stone()
{
    TEST_CASE("Place Stone");

    Board board;
    board_init(&board);

    // 첫 번째 돌 놓기
    bool result = board_place_stone(&board, 9, 9, BLACK);
    TEST_ASSERT_TRUE(result, "Should successfully place first stone");
    TEST_ASSERT_EQ(board_get_stone(&board, 9, 9), BLACK, "Stone at (9,9) should be BLACK");
    TEST_ASSERT_EQ(board.move_count, 1, "Move count should be 1");
    TEST_ASSERT_EQ(board.last_row, 9, "Last row should be 9");
    TEST_ASSERT_EQ(board.last_col, 9, "Last col should be 9");

    // 같은 위치에 다시 놓기 (실패해야 함)
    result = board_place_stone(&board, 9, 9, WHITE);
    TEST_ASSERT_FALSE(result, "Should not place stone on occupied position");
    TEST_ASSERT_EQ(board.move_count, 1, "Move count should still be 1");

    // 다른 위치에 놓기
    result = board_place_stone(&board, 9, 10, WHITE);
    TEST_ASSERT_TRUE(result, "Should successfully place second stone");
    TEST_ASSERT_EQ(board_get_stone(&board, 9, 10), WHITE, "Stone at (9,10) should be WHITE");
    TEST_ASSERT_EQ(board.move_count, 2, "Move count should be 2");
}

// 범위 밖 테스트
void test_board_out_of_bounds()
{
    TEST_CASE("Out of Bounds Check");

    Board board;
    board_init(&board);

    // 범위 밖 좌표
    bool result = board_place_stone(&board, -1, 5, BLACK);
    TEST_ASSERT_FALSE(result, "Should reject negative row");

    result = board_place_stone(&board, 5, -1, BLACK);
    TEST_ASSERT_FALSE(result, "Should reject negative col");

    result = board_place_stone(&board, BOARD_SIZE, 5, BLACK);
    TEST_ASSERT_FALSE(result, "Should reject row >= BOARD_SIZE");

    result = board_place_stone(&board, 5, BOARD_SIZE, BLACK);
    TEST_ASSERT_FALSE(result, "Should reject col >= BOARD_SIZE");
}

// 빈 칸 체크 테스트
void test_board_is_empty()
{
    TEST_CASE("Is Empty Check");

    Board board;
    board_init(&board);

    TEST_ASSERT_TRUE(board_is_empty(&board, 5, 5), "Position (5,5) should be empty");

    board_place_stone(&board, 5, 5, BLACK);
    TEST_ASSERT_FALSE(board_is_empty(&board, 5, 5), "Position (5,5) should not be empty");

    TEST_ASSERT_TRUE(board_is_empty(&board, 5, 6), "Position (5,6) should still be empty");
}

// 금수 마크 업데이트 테스트
void test_board_forbidden_marks()
{
    TEST_CASE("Forbidden Marks Update");

    Board board;
    board_init_with_rule(&board, RULE_RENJU);

    // 초기에는 모든 금수 마크가 false
    bool all_not_forbidden = true;
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            if (board.forbidden_marks[row][col])
            {
                all_not_forbidden = false;
                break;
            }
        }
    }
    TEST_ASSERT_TRUE(all_not_forbidden, "Initially no forbidden marks");

    // 금수 마크 업데이트 (실제 금수가 생기는지는 rules 테스트에서 확인)
    board_update_forbidden_marks(&board, BLACK);
    TEST_ASSERT(true, "Forbidden marks update should complete without error");
}

// 보드 상태 복사 테스트 (나중에 무르기 기능에 필요)
void test_board_get_move_count()
{
    TEST_CASE("Get Move Count");

    Board board;
    board_init(&board);

    TEST_ASSERT_EQ(board_get_move_count(&board), 0, "Initial move count is 0");

    board_place_stone(&board, 0, 0, BLACK);
    TEST_ASSERT_EQ(board_get_move_count(&board), 1, "Move count after first move is 1");

    board_place_stone(&board, 0, 1, WHITE);
    TEST_ASSERT_EQ(board_get_move_count(&board), 2, "Move count after second move is 2");
}

// 모든 보드 테스트 실행
void run_board_tests()
{
    TEST_SUITE("Board Logic Tests");

    test_board_init();
    test_board_init_with_rule();
    test_board_place_stone();
    test_board_out_of_bounds();
    test_board_is_empty();
    test_board_forbidden_marks();
    test_board_get_move_count();
}

int main()
{
    test_init();
    run_board_tests();
    test_summary();

    return (g_test_stats.failed == 0) ? 0 : 1;
}
