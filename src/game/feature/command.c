#include "command.h"
#include <string.h>
#include <stdio.h>

bool command_is_command(const char *message)
{
    return message != NULL && message[0] == '/';
}

CommandResult command_parse(const char *message)
{
    CommandResult result;
    memset(&result, 0, sizeof(result));

    if (!command_is_command(message))
    {
        result.type = CMD_NONE;
        result.valid = false;
        strcpy(result.error_message, "Not a command");
        return result;
    }

    if (strcmp(message, "/quit") == 0)
    {
        result.type = CMD_QUIT;
        result.valid = true;
    }
    else if (strcmp(message, "/undo") == 0)
    {
        result.type = CMD_UNDO;
        result.valid = true;
    }
    else if (strcmp(message, "/giveup") == 0)
    {
        result.type = CMD_GIVEUP;
        result.valid = true;
    }
    else if (strcmp(message, "/swap") == 0)
    {
        result.type = CMD_SWAP;
        result.valid = true;
    }
    else if (strcmp(message, "/help") == 0)
    {
        result.type = CMD_HELP;
        result.valid = true;
    }
    else
    {
        result.type = CMD_INVALID;
        result.valid = false;
        snprintf(result.error_message, sizeof(result.error_message),
                 "Unknown command: %s", message);
    }

    return result;
}
