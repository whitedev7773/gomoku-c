#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include "../core/board.h"
#include <stdbool.h>

typedef enum
{
    AI_EASY = 0,
    AI_HARD = 1
} AIDifficulty;

typedef struct
{
    AIDifficulty difficulty;
    Stone ai_stone; // AI가 사용하는 돌 색상
} AIEngine;

/**
 * AI 엔진 초기화
 */
void ai_init(AIEngine *ai, AIDifficulty difficulty, Stone ai_stone);

/**
 * AI가 다음 수를 계산
 * @return 성공 시 true, 실패 시 false
 */
bool ai_get_next_move(const AIEngine *ai, const Board *board, Position *move);

/**
 * Easy AI - 기본 휴리스틱 알고리즘
 */
bool ai_easy_get_move(const Board *board, Stone ai_stone, Position *move);

/**
 * Hard AI - Minimax + Alpha-Beta Pruning
 */
bool ai_hard_get_move(const Board *board, Stone ai_stone, Position *move);

#endif // AI_ENGINE_H
