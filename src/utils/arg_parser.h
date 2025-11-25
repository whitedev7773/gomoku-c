#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include <stdbool.h>

typedef enum {
    MODE_MENU,              // Default: Show menu
    MODE_SINGLEPLAY_EASY,   // --singleplay --easy
    MODE_SINGLEPLAY_HARD,   // --singleplay --hard
    MODE_MULTIPLAY_HOST,    // --multiplay-host
    MODE_MULTIPLAY_CLIENT,  // --multiplay-client -ip <IP> -port <PORT>
    MODE_SPECTATOR          // --spectator -ip <IP> -port <PORT>
} GameMode;

typedef struct {
    GameMode mode;
    char ip_address[16];    // For client/spectator mode
    int port;               // For client/spectator mode (default: 7773)
    bool valid;
    char error_message[256];
} ParsedArgs;

// Parse command line arguments
ParsedArgs parse_arguments(int argc, char *argv[]);

// Display usage information
void display_usage(const char *program_name);

#endif // ARG_PARSER_H
