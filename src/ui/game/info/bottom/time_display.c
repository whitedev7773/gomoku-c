#include "time_display.h"
#include "../../../core/theme.h"
#include <stdio.h>

#define TURN_TIMEOUT_BAR_WIDTH 20

void time_display_init(TimeDisplay *display)
{
    if (!display)
        return;
    display->game_start_time = time(NULL);
    display->prev_remaining_seconds = -1;
    display->prev_elapsed_seconds = -1;
}

int time_display_elapsed_sec(const TimeDisplay *display)
{
    if (!display)
        return 0;
    time_t now = time(NULL);
    return (int)difftime(now, display->game_start_time);
}

void time_display_draw_timer(WINDOW *win, int remaining_seconds)
{
    if (!win)
        return;

    // TIMER 영역: (32,24) 시작, 48x3
    // 내부 좌표: (33,25) ~ (79,26)
    int filled = remaining_seconds;
    if (filled > TURN_TIMEOUT_SECONDS)
        filled = TURN_TIMEOUT_SECONDS;
    if (filled < 0)
        filled = 0;

    attron(COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvprintw(25, 35, "TIME: %2ds", remaining_seconds);
    attroff(COLOR_PAIR(COLOR_PAIR_DEFAULT));

    attron(COLOR_PAIR(COLOR_PAIR_DEFAULT));
    for (int i = 0; i < filled; i++)
    {
        mvprintw(25, 46 + i, "█");
    }
    for (int i = filled; i < TURN_TIMEOUT_SECONDS; i++)
    {
        mvprintw(25, 46 + i, "░");
    }
    attroff(COLOR_PAIR(COLOR_PAIR_DEFAULT));
    refresh();
}

void time_display_draw_playtime(WINDOW *win, const TimeDisplay *display)
{
    if (!win || !display)
        return;

    // PLAY TIME 영역: (79,24) 시작, 21x3
    // 내부 좌표: (80,25) ~ (99,26)
    int elapsed = time_display_elapsed_sec(display);
    int minutes = elapsed / 60;
    int seconds = elapsed % 60;

    attron(COLOR_PAIR(COLOR_PAIR_DEFAULT) | A_BOLD);
    mvprintw(25, 79, "┃");
    attroff(COLOR_PAIR(COLOR_PAIR_DEFAULT) | A_BOLD);

    attron(COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvprintw(25, 81, "PLAY TIME:  %02d:%02d ", minutes, seconds);
    attroff(COLOR_PAIR(COLOR_PAIR_DEFAULT));
    refresh();
}

bool time_display_render_timer(WINDOW *win, const TurnManager *turn_mgr,
                               TimeDisplay *display,
                               UIRenderFlags *flags,
                               bool force_render)
{
    if (!win || !turn_mgr || !display || !flags)
        return false;

    bool rendered = false;

    if (force_render || ui_render_flags_is_set(flags, RENDER_TIMER))
    {
        int remaining = turn_manager_get_remaining_time(turn_mgr);

        // 변경 여부 확인 (초 단위)
        if (force_render || display->prev_remaining_seconds != remaining)
        {
            time_display_draw_timer(win, remaining);
            display->prev_remaining_seconds = remaining;
            rendered = true;
        }

        ui_render_flags_clear(flags, RENDER_TIMER);
    }

    return rendered;
}

bool time_display_render_playtime(WINDOW *win,
                                  TimeDisplay *display,
                                  UIRenderFlags *flags,
                                  bool force_render)
{
    if (!win || !display || !flags)
        return false;

    bool rendered = false;

    if (force_render || ui_render_flags_is_set(flags, RENDER_PLAY_TIME))
    {
        int elapsed = time_display_elapsed_sec(display);

        // 변경 여부 확인 (초 단위)
        if (force_render || display->prev_elapsed_seconds != elapsed)
        {
            time_display_draw_playtime(win, display);
            display->prev_elapsed_seconds = elapsed;
            rendered = true;
        }

        ui_render_flags_clear(flags, RENDER_PLAY_TIME);
    }

    return rendered;
}
