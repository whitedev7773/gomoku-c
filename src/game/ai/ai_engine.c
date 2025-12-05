#include "ai_engine.h"
#include "../core/game_logic.h"
#include <stdlib.h>
#include <limits.h>
#include <string.h>

// 평가 점수 상수
#define SCORE_FIVE 100000     // 5개 연속 (승리)
#define SCORE_OPEN_FOUR 10000 // 양쪽 열린 4개
#define SCORE_FOUR 5000       // 한쪽 막힌 4개
#define SCORE_OPEN_THREE 1000 // 양쪽 열린 3개
#define SCORE_THREE 500       // 한쪽 막힌 3개
#define SCORE_OPEN_TWO 100    // 양쪽 열린 2개
#define SCORE_TWO 50          // 한쪽 막힌 2개
#define SCORE_ONE 10          // 1개

// Minimax 깊이 제한
#define MAX_DEPTH 3

// 8방향 (가로, 세로, 대각선)
static const int dx[] = {0, 1, 1, 1, 0, -1, -1, -1};
static const int dy[] = {1, 1, 0, -1, -1, -1, 0, 1};

void ai_init(AIEngine *ai, AIDifficulty difficulty, Stone ai_stone)
{
    ai->difficulty = difficulty;
    ai->ai_stone = ai_stone;
}

bool ai_get_next_move(const AIEngine *ai, const Board *board, Position *move)
{
    if (ai->difficulty == AI_EASY)
    {
        return ai_easy_get_move(board, ai->ai_stone, move);
    }
    else
    {
        return ai_hard_get_move(board, ai->ai_stone, move);
    }
}

// 특정 방향으로 연속된 돌의 개수 세기
static int count_consecutive(const Board *board, int row, int col, int dir, Stone stone)
{
    int count = 0;
    int r = row + dx[dir];
    int c = col + dy[dir];

    while (board_is_valid_position(r, c) && board_get_stone(board, r, c) == stone)
    {
        count++;
        r += dx[dir];
        c += dy[dir];
    }

    return count;
}

// 특정 위치에 돌을 놓았을 때 만들어지는 패턴 평가
static int evaluate_position(const Board *board, int row, int col, Stone stone)
{
    int score = 0;

    // 4방향 체크 (가로, 세로, 대각선 2개)
    for (int dir = 0; dir < 4; dir++)
    {
        int forward = count_consecutive(board, row, col, dir, stone);
        int backward = count_consecutive(board, row, col, dir + 4, stone);
        int total = forward + backward + 1;

        // 양쪽 끝이 막혔는지 체크
        int fr = row + dx[dir] * (forward + 1);
        int fc = col + dy[dir] * (forward + 1);
        int br = row + dx[dir + 4] * (backward + 1);
        int bc = col + dy[dir + 4] * (backward + 1);

        bool forward_blocked = !board_is_valid_position(fr, fc) ||
                               (board_get_stone(board, fr, fc) != EMPTY &&
                                board_get_stone(board, fr, fc) != stone);
        bool backward_blocked = !board_is_valid_position(br, bc) ||
                                (board_get_stone(board, br, bc) != EMPTY &&
                                 board_get_stone(board, br, bc) != stone);

        // 점수 계산
        if (total >= 5)
        {
            score += SCORE_FIVE;
        }
        else if (total == 4)
        {
            if (!forward_blocked && !backward_blocked)
            {
                score += SCORE_OPEN_FOUR;
            }
            else if (!forward_blocked || !backward_blocked)
            {
                score += SCORE_FOUR;
            }
        }
        else if (total == 3)
        {
            if (!forward_blocked && !backward_blocked)
            {
                score += SCORE_OPEN_THREE;
            }
            else if (!forward_blocked || !backward_blocked)
            {
                score += SCORE_THREE;
            }
        }
        else if (total == 2)
        {
            if (!forward_blocked && !backward_blocked)
            {
                score += SCORE_OPEN_TWO;
            }
            else if (!forward_blocked || !backward_blocked)
            {
                score += SCORE_TWO;
            }
        }
        else if (total == 1)
        {
            score += SCORE_ONE;
        }
    }

    return score;
}

// Easy AI - 휴리스틱 기반
bool ai_easy_get_move(const Board *board, Stone ai_stone, Position *move)
{
    Stone opponent_stone = (ai_stone == BLACK) ? WHITE : BLACK;
    int best_score = -1;
    Position best_move = {-1, -1};

    // 첫 수인 경우 중앙에 두기
    if (board_get_move_count(board) == 0)
    {
        move->row = BOARD_SIZE / 2;
        move->col = BOARD_SIZE / 2;
        return true;
    }

    // 모든 빈 칸을 검사
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            if (!board_is_empty(board, row, col))
            {
                continue;
            }

            int score = 0;

            // AI의 공격 점수
            int attack_score = evaluate_position(board, row, col, ai_stone);

            // 상대방의 위협 점수 (방어)
            int defense_score = evaluate_position(board, row, col, opponent_stone);

            // 방어를 약간 더 우선시 (상대방의 승리를 막는 것이 중요)
            score = attack_score + defense_score * 1.1;

            // 중앙 근처 보너스
            int center_dist = abs(row - BOARD_SIZE / 2) + abs(col - BOARD_SIZE / 2);
            score += (BOARD_SIZE - center_dist) * 2;

            if (score > best_score)
            {
                best_score = score;
                best_move.row = row;
                best_move.col = col;
            }
        }
    }

    if (best_move.row != -1)
    {
        *move = best_move;
        return true;
    }

    return false;
}

// 보드 전체 평가 함수
static int evaluate_board(const Board *board, Stone ai_stone)
{
    Stone opponent_stone = (ai_stone == BLACK) ? WHITE : BLACK;
    int score = 0;

    // 게임 종료 체크
    GameResult result = game_check_winner(board);
    if (result == GAME_BLACK_WIN)
    {
        return (ai_stone == BLACK) ? SCORE_FIVE * 10 : -SCORE_FIVE * 10;
    }
    else if (result == GAME_WHITE_WIN)
    {
        return (ai_stone == WHITE) ? SCORE_FIVE * 10 : -SCORE_FIVE * 10;
    }

    // 모든 돌에 대해 평가
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            Stone stone = board_get_stone(board, row, col);
            if (stone == ai_stone)
            {
                score += evaluate_position(board, row, col, ai_stone) / 4;
            }
            else if (stone == opponent_stone)
            {
                score -= evaluate_position(board, row, col, opponent_stone) / 4;
            }
        }
    }

    return score;
}

// Minimax with Alpha-Beta Pruning
static int minimax(Board *board, int depth, bool is_maximizing, Stone ai_stone,
                   int alpha, int beta, Position *best_move)
{
    // 종료 조건
    if (depth == 0 || game_is_over(board))
    {
        return evaluate_board(board, ai_stone);
    }

    Stone current_stone = is_maximizing ? ai_stone : ((ai_stone == BLACK) ? WHITE : BLACK);

    if (is_maximizing)
    {
        int max_eval = INT_MIN;
        Position local_best = {-1, -1};

        // 모든 가능한 수 탐색 (최적화: 이미 돌이 있는 곳 주변만)
        for (int row = 0; row < BOARD_SIZE; row++)
        {
            for (int col = 0; col < BOARD_SIZE; col++)
            {
                if (!board_is_empty(board, row, col))
                {
                    continue;
                }

                // 주변에 돌이 있는지 체크 (가지치기 최적화)
                bool has_neighbor = false;
                if (board_get_move_count(board) == 0)
                {
                    has_neighbor = (row >= BOARD_SIZE / 2 - 2 && row <= BOARD_SIZE / 2 + 2 &&
                                    col >= BOARD_SIZE / 2 - 2 && col <= BOARD_SIZE / 2 + 2);
                }
                else
                {
                    for (int d = 0; d < 8; d++)
                    {
                        int nr = row + dx[d];
                        int nc = col + dy[d];
                        if (board_is_valid_position(nr, nc) && !board_is_empty(board, nr, nc))
                        {
                            has_neighbor = true;
                            break;
                        }
                    }
                }

                if (!has_neighbor)
                    continue;

                // 수 두기
                board_place_stone(board, row, col, current_stone);

                int eval = minimax(board, depth - 1, false, ai_stone, alpha, beta, NULL);

                // 수 되돌리기 (직접 셀 조작으로 최적화)
                board->cells[row][col] = EMPTY;
                board->move_count--;
                board->history.count--;

                if (eval > max_eval)
                {
                    max_eval = eval;
                    local_best.row = row;
                    local_best.col = col;
                }

                alpha = (alpha > eval) ? alpha : eval;
                if (beta <= alpha)
                {
                    break; // Beta cut-off
                }
            }
            if (beta <= alpha)
                break;
        }

        if (best_move && local_best.row != -1)
        {
            *best_move = local_best;
        }

        return max_eval;
    }
    else
    {
        int min_eval = INT_MAX;

        for (int row = 0; row < BOARD_SIZE; row++)
        {
            for (int col = 0; col < BOARD_SIZE; col++)
            {
                if (!board_is_empty(board, row, col))
                {
                    continue;
                }

                // 주변 체크 (위와 동일)
                bool has_neighbor = false;
                if (board_get_move_count(board) == 0)
                {
                    has_neighbor = (row >= BOARD_SIZE / 2 - 2 && row <= BOARD_SIZE / 2 + 2 &&
                                    col >= BOARD_SIZE / 2 - 2 && col <= BOARD_SIZE / 2 + 2);
                }
                else
                {
                    for (int d = 0; d < 8; d++)
                    {
                        int nr = row + dx[d];
                        int nc = col + dy[d];
                        if (board_is_valid_position(nr, nc) && !board_is_empty(board, nr, nc))
                        {
                            has_neighbor = true;
                            break;
                        }
                    }
                }

                if (!has_neighbor)
                    continue;

                board_place_stone(board, row, col, current_stone);

                int eval = minimax(board, depth - 1, true, ai_stone, alpha, beta, NULL);

                // 수 되돌리기 (직접 셀 조작으로 최적화)
                board->cells[row][col] = EMPTY;
                board->move_count--;
                board->history.count--;

                min_eval = (min_eval < eval) ? min_eval : eval;
                beta = (beta < eval) ? beta : eval;
                if (beta <= alpha)
                {
                    break; // Alpha cut-off
                }
            }
            if (beta <= alpha)
                break;
        }

        return min_eval;
    }
}

// Hard AI - Minimax + Alpha-Beta Pruning
bool ai_hard_get_move(const Board *board, Stone ai_stone, Position *move)
{
    // 첫 수인 경우 중앙에 두기
    if (board_get_move_count(board) == 0)
    {
        move->row = BOARD_SIZE / 2;
        move->col = BOARD_SIZE / 2;
        return true;
    }

    // 보드 복사 (minimax에서 수정하므로)
    Board temp_board;
    memcpy(&temp_board, board, sizeof(Board));

    Position best_move = {-1, -1};
    minimax(&temp_board, MAX_DEPTH, true, ai_stone, INT_MIN, INT_MAX, &best_move);

    if (best_move.row != -1)
    {
        *move = best_move;
        return true;
    }

    // fallback: easy AI 사용
    return ai_easy_get_move(board, ai_stone, move);
}
