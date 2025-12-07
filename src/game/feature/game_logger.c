#include "game_logger.h"
#include <string.h>
#include <time.h>

void logger_generate_filename(char *filename, size_t size)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    snprintf(filename, size, "gomoku-%04d%02d%02d-%02d:%02d.log",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min);
}

bool logger_init(GameLogger *logger)
{
    if (!logger)
        return false;

    memset(logger, 0, sizeof(GameLogger));
    logger->entry_count = 0;
    logger->game_start_time = time(NULL);

    logger_generate_filename(logger->filename, LOG_FILENAME_SIZE);

    logger->file = fopen(logger->filename, "w");
    if (!logger->file)
    {
        return false;
    }

    struct tm *t = localtime(&logger->game_start_time);
    fprintf(logger->file, "=== GOMOKU GAME LOG ===\n");
    fprintf(logger->file, "Game started: %04d-%02d-%02d %02d:%02d:%02d\n\n",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);

    return true;
}

void logger_close(GameLogger *logger)
{
    if (!logger)
        return;

    if (logger->file)
    {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        fprintf(logger->file, "\n=== GAME END ===\n");
        fprintf(logger->file, "Game ended: %04d-%02d-%02d %02d:%02d:%02d\n",
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min, t->tm_sec);
        fclose(logger->file);
        logger->file = NULL;
    }
}

bool logger_log_move(GameLogger *logger, Stone player, int row, int col, int move_number)
{
    if (!logger || !logger->file)
        return false;

    if (logger->entry_count >= MAX_LOG_ENTRIES)
    {
        return false;
    }

    LogEntry *entry = &logger->entries[logger->entry_count];
    entry->player = player;
    entry->row = row;
    entry->col = col;
    entry->timestamp = time(NULL);
    entry->move_number = move_number;

    logger->entry_count++;

    const char *player_str = (player == BLACK) ? "BLACK" : "WHITE";
    char col_char = 'A' + col;

    fprintf(logger->file, "Move %3d: %s placed at %c%02d\n",
            move_number, player_str, col_char, row);
    fflush(logger->file);

    return true;
}

bool logger_log_event(GameLogger *logger, const char *event_description)
{
    if (!logger || !logger->file || !event_description)
    {
        return false;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(logger->file, "[%02d:%02d:%02d] %s\n",
            t->tm_hour, t->tm_min, t->tm_sec, event_description);
    fflush(logger->file);

    return true;
}

bool logger_save_to_file(GameLogger *logger)
{
    if (!logger || !logger->file)
        return false;

    fflush(logger->file);
    return true;
}

bool logger_load_from_file(GameLogger *logger, const char *filename)
{
    if (!logger || !filename)
        return false;

    memset(logger, 0, sizeof(GameLogger));
    strncpy(logger->filename, filename, LOG_FILENAME_SIZE - 1);

    FILE *file = fopen(filename, "r");
    if (!file)
        return false;

    char line[256];
    int entry_count = 0;

    while (fgets(line, sizeof(line), file) && entry_count < MAX_LOG_ENTRIES)
    {
        // 로그 라인 파싱: "Move XXX: BLACK/WHITE placed at A00"
        int move_num;
        char player_str[16];
        char col_char;
        int row;

        if (sscanf(line, "Move %d: %15s placed at %c%d", &move_num, player_str, &col_char, &row) == 4)
        {
            LogEntry *entry = &logger->entries[entry_count];
            entry->move_number = move_num;
            entry->player = (strcmp(player_str, "BLACK") == 0) ? BLACK : WHITE;
            entry->col = col_char - 'A';
            entry->row = row;
            entry->timestamp = 0; // 타임스탬프는 재생 시 무시

            entry_count++;
        }
    }

    logger->entry_count = entry_count;
    fclose(file);

    return entry_count > 0;
}
