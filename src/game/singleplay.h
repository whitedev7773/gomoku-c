#ifndef SINGLEPLAY_H
#define SINGLEPLAY_H

#include "ai_engine.h"
#include "board.h"

/**
 * 싱글플레이 게임 실행
 * @param difficulty AI 난이도
 * @param rule 게임 규칙 (RULE_STANDARD 또는 RULE_RENJU)
 * @return 게임 종료 시 0, 오류 시 -1
 */
int singleplay_run(AIDifficulty difficulty, GameRule rule);

#endif // SINGLEPLAY_H
