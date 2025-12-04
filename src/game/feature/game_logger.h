#ifndef GAME_LOGGER_H
#define GAME_LOGGER_H

#include "../core/board.h"
#include <stdio.h>
#include <time.h>

#define MAX_LOG_ENTRIES 400
#define LOG_FILENAME_SIZE 64

typedef struct
{
    Stone player;
    int row;
    int col;
    time_t timestamp;
    int move_number;
} LogEntry;

typedef struct
{
    LogEntry entries[MAX_LOG_ENTRIES];
    int entry_count;
    char filename[LOG_FILENAME_SIZE];
    FILE *file;
    time_t game_start_time;
} GameLogger;

bool logger_init(GameLogger *logger);

void logger_close(GameLogger *logger);

bool logger_log_move(GameLogger *logger, Stone player, int row, int col, int move_number);

bool logger_log_event(GameLogger *logger, const char *event_description);

bool logger_save_to_file(GameLogger *logger);

void logger_generate_filename(char *filename, size_t size);

bool logger_load_from_file(GameLogger *logger, const char *filename);

void logger_print_entry(const LogEntry *entry);

#endif // GAME_LOGGER_H
