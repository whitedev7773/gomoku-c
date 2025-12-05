#include "rules.h"
#include "game_logic.h"

static int count_consecutive(const Board *board, int row, int col, Stone stone, int dr, int dc)
{
    int count = 0;
    int r = row + dr;
    int c = col + dc;

    while (board_is_valid_position(r, c) && board_get_stone(board, r, c) == stone)
    {
        count++;
        r += dr;
        c += dc;
    }

    return count;
}

static int count_total_in_direction(const Board *board, int row, int col, Stone stone, int dr, int dc)
{
    int count = 1;
    count += count_consecutive(board, row, col, stone, dr, dc);
    count += count_consecutive(board, row, col, stone, -dr, -dc);
    return count;
}

static bool is_open_three(const Board *board, int row, int col, Stone stone, int dr, int dc)
{
    // const를 우회하여 임시로 셀 수정 (최적화)
    Board *mutable_board = (Board *)board;
    Stone original = mutable_board->cells[row][col];
    mutable_board->cells[row][col] = stone;

    int count = count_total_in_direction(board, row, col, stone, dr, dc);

    if (count != 3)
    {
        mutable_board->cells[row][col] = original;
        return false;
    }

    int forward = count_consecutive(board, row, col, stone, dr, dc);
    int backward = count_consecutive(board, row, col, stone, -dr, -dc);

    int end_r = row + dr * (forward + 1);
    int end_c = col + dc * (forward + 1);
    int start_r = row - dr * (backward + 1);
    int start_c = col - dc * (backward + 1);

    bool open_end = board_is_valid_position(end_r, end_c) &&
                    board_get_stone(board, end_r, end_c) == EMPTY;
    bool open_start = board_is_valid_position(start_r, start_c) &&
                      board_get_stone(board, start_r, start_c) == EMPTY;

    mutable_board->cells[row][col] = original;
    return (open_end && open_start);
}

static bool is_open_four(const Board *board, int row, int col, Stone stone, int dr, int dc)
{
    // const를 우회하여 임시로 셀 수정 (최적화)
    Board *mutable_board = (Board *)board;
    Stone original = mutable_board->cells[row][col];
    mutable_board->cells[row][col] = stone;

    int count = count_total_in_direction(board, row, col, stone, dr, dc);

    if (count != 4)
    {
        mutable_board->cells[row][col] = original;
        return false;
    }

    int forward = count_consecutive(board, row, col, stone, dr, dc);
    int backward = count_consecutive(board, row, col, stone, -dr, -dc);

    int end_r = row + dr * (forward + 1);
    int end_c = col + dc * (forward + 1);
    int start_r = row - dr * (backward + 1);
    int start_c = col - dc * (backward + 1);

    bool open_end = board_is_valid_position(end_r, end_c) &&
                    board_get_stone(board, end_r, end_c) == EMPTY;
    bool open_start = board_is_valid_position(start_r, start_c) &&
                      board_get_stone(board, start_r, start_c) == EMPTY;

    mutable_board->cells[row][col] = original;
    return (open_end || open_start);
}

bool rules_check_double_three(const Board *board, int row, int col, Stone stone)
{
    if (!board || stone != BLACK)
        return false;

    int three_count = 0;
    int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};

    for (int i = 0; i < 4; i++)
    {
        if (is_open_three(board, row, col, stone, directions[i][0], directions[i][1]))
        {
            three_count++;
        }
    }

    return (three_count >= 2);
}

bool rules_check_double_four(const Board *board, int row, int col, Stone stone)
{
    if (!board || stone != BLACK)
        return false;

    int four_count = 0;
    int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};

    for (int i = 0; i < 4; i++)
    {
        if (is_open_four(board, row, col, stone, directions[i][0], directions[i][1]))
        {
            four_count++;
        }
    }

    return (four_count >= 2);
}

bool rules_check_overline(const Board *board, int row, int col, Stone stone)
{
    if (!board || stone != BLACK)
        return false;

    // const를 우회하여 임시로 셀 수정 (최적화)
    Board *mutable_board = (Board *)board;
    Stone original = mutable_board->cells[row][col];
    mutable_board->cells[row][col] = stone;

    int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
    bool result = false;

    for (int i = 0; i < 4; i++)
    {
        int count = count_total_in_direction(board, row, col, stone,
                                             directions[i][0], directions[i][1]);
        if (count > 5)
        {
            result = true;
            break;
        }
    }

    mutable_board->cells[row][col] = original;
    return result;
}

bool rules_is_forbidden_move(const Board *board, int row, int col, Stone stone,
                             GameRule rule, ForbiddenType *forbidden_type)
{
    if (!board || stone != BLACK || rule != RULE_RENJU)
    {
        if (forbidden_type)
            *forbidden_type = FORBIDDEN_NONE;
        return false;
    }

    if (rules_check_overline(board, row, col, stone))
    {
        if (forbidden_type)
            *forbidden_type = FORBIDDEN_OVERLINE;
        return true;
    }

    if (rules_check_double_four(board, row, col, stone))
    {
        if (forbidden_type)
            *forbidden_type = FORBIDDEN_DOUBLE_FOUR;
        return true;
    }

    if (rules_check_double_three(board, row, col, stone))
    {
        if (forbidden_type)
            *forbidden_type = FORBIDDEN_DOUBLE_THREE;
        return true;
    }

    if (forbidden_type)
        *forbidden_type = FORBIDDEN_NONE;
    return false;
}

bool rules_is_swap_available(const Board *board)
{
    if (!board)
        return false;
    return (board_get_move_count(board) == 3);
}

void rules_apply_swap(Board *board)
{
    if (!board || !rules_is_swap_available(board))
    {
        return;
    }

    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board->cells[i][j] == BLACK)
            {
                board->cells[i][j] = WHITE;
            }
            else if (board->cells[i][j] == WHITE)
            {
                board->cells[i][j] = BLACK;
            }
        }
    }
}
