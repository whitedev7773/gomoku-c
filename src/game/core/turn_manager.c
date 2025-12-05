#include "turn_manager.h"
#include <time.h>

void turn_manager_init(TurnManager *manager, Stone first_player)
{
    if (!manager)
        return;

    manager->current_player = first_player;
    manager->turn_start_time = time(NULL);
    manager->total_turns = 0;
    manager->timeout_enabled = true;
    manager->paused = false;
    manager->paused_remaining = TURN_TIMEOUT_SECONDS;
}

Stone turn_manager_get_current_player(const TurnManager *manager)
{
    if (!manager)
        return EMPTY;
    return manager->current_player;
}

void turn_manager_next_turn(TurnManager *manager)
{
    if (!manager)
        return;

    manager->current_player = turn_manager_get_opposite_player(manager->current_player);
    manager->turn_start_time = time(NULL);
    manager->total_turns++;
}

bool turn_manager_is_timeout(const TurnManager *manager)
{
    if (!manager || !manager->timeout_enabled || manager->paused)
    {
        return false;
    }

    time_t current_time = time(NULL);
    double elapsed = difftime(current_time, manager->turn_start_time);

    return (elapsed >= TURN_TIMEOUT_SECONDS);
}

int turn_manager_get_remaining_time(const TurnManager *manager)
{
    if (!manager)
        return 0;

    // 일시정지 상태면 저장된 남은 시간 반환
    if (manager->paused)
    {
        return manager->paused_remaining;
    }

    time_t current_time = time(NULL);
    double elapsed = difftime(current_time, manager->turn_start_time);
    int remaining = TURN_TIMEOUT_SECONDS - (int)elapsed;

    return (remaining > 0) ? remaining : 0;
}

void turn_manager_reset_timer(TurnManager *manager)
{
    if (!manager)
        return;
    manager->turn_start_time = time(NULL);
}

Stone turn_manager_get_opposite_player(Stone player)
{
    if (player == BLACK)
        return WHITE;
    if (player == WHITE)
        return BLACK;
    return EMPTY;
}

void turn_manager_set_timeout_enabled(TurnManager *manager, bool enabled)
{
    if (!manager)
        return;
    manager->timeout_enabled = enabled;
}

void turn_manager_pause(TurnManager *manager)
{
    if (!manager || manager->paused)
        return;

    // 현재 남은 시간 저장
    manager->paused_remaining = turn_manager_get_remaining_time(manager);
    manager->paused = true;
}

void turn_manager_resume(TurnManager *manager)
{
    if (!manager || !manager->paused)
        return;

    // 저장된 남은 시간 기준으로 시작 시간 재설정
    time_t now = time(NULL);
    manager->turn_start_time = now - (TURN_TIMEOUT_SECONDS - manager->paused_remaining);
    manager->paused = false;
}

bool turn_manager_is_paused(const TurnManager *manager)
{
    if (!manager)
        return false;
    return manager->paused;
}
