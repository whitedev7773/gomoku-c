#include "arg_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_PORT 7773

void display_usage(const char *program_name) {
    printf("\n");
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("\n");
    printf("Options:\n");
    printf("  (no arguments)                                Start with menu\n");
    printf("  --singleplay --easy                           Start singleplay with Easy AI\n");
    printf("  --singleplay --hard                           Start singleplay with Hard AI\n");
    printf("  --multiplay-host                              Start multiplayer as host\n");
    printf("  --multiplay-client -ip <IP> [-port <PORT>]    Join multiplayer game at <IP>:<PORT>\n");
    printf("  --spectator -ip <IP> [-port <PORT>]           Spectate game at <IP>:<PORT>\n");
    printf("  --help                                        Display this help message\n");
    printf("\n");
    printf("Note: Default port is %d if not specified\n", DEFAULT_PORT);
    printf("\n");
    printf("Examples:\n");
    printf("  %s --singleplay --easy\n", program_name);
    printf("  %s --multiplay-host\n", program_name);
    printf("  %s --multiplay-client -ip 192.168.0.2\n", program_name);
    printf("  %s --multiplay-client -ip 192.168.0.2 -port 9000\n", program_name);
    printf("  %s --spectator -ip 192.168.0.2 -port 8888\n", program_name);
    printf("\n");
}

ParsedArgs parse_arguments(int argc, char *argv[]) {
    ParsedArgs args = {
        .mode = MODE_MENU,
        .valid = true,
        .ip_address = {0},
        .port = DEFAULT_PORT,
        .error_message = {0}
    };

    // No arguments - show menu
    if (argc == 1) {
        return args;
    }

    // Help option
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        display_usage(argv[0]);
        args.valid = false;
        return args;
    }

    // Singleplay mode
    if (strcmp(argv[1], "--singleplay") == 0) {
        if (argc < 3) {
            snprintf(args.error_message, sizeof(args.error_message),
                    "Error: --singleplay requires difficulty (--easy or --hard)");
            args.valid = false;
            return args;
        }

        if (strcmp(argv[2], "--easy") == 0) {
            args.mode = MODE_SINGLEPLAY_EASY;
        } else if (strcmp(argv[2], "--hard") == 0) {
            args.mode = MODE_SINGLEPLAY_HARD;
        } else {
            snprintf(args.error_message, sizeof(args.error_message),
                    "Error: Invalid difficulty '%s'. Use --easy or --hard", argv[2]);
            args.valid = false;
        }
        return args;
    }

    // Multiplay host mode
    if (strcmp(argv[1], "--multiplay-host") == 0) {
        args.mode = MODE_MULTIPLAY_HOST;
        return args;
    }

    // Multiplay client mode
    if (strcmp(argv[1], "--multiplay-client") == 0) {
        if (argc < 4 || strcmp(argv[2], "-ip") != 0) {
            snprintf(args.error_message, sizeof(args.error_message),
                    "Error: --multiplay-client requires -ip <IP>");
            args.valid = false;
            return args;
        }

        args.mode = MODE_MULTIPLAY_CLIENT;
        strncpy(args.ip_address, argv[3], sizeof(args.ip_address) - 1);

        // Check for optional -port argument
        if (argc >= 6 && strcmp(argv[4], "-port") == 0) {
            args.port = atoi(argv[5]);
            if (args.port <= 0 || args.port > 65535) {
                snprintf(args.error_message, sizeof(args.error_message),
                        "Error: Invalid port number '%s'. Port must be between 1 and 65535", argv[5]);
                args.valid = false;
                return args;
            }
        }
        return args;
    }

    // Spectator mode
    if (strcmp(argv[1], "--spectator") == 0) {
        if (argc < 4 || strcmp(argv[2], "-ip") != 0) {
            snprintf(args.error_message, sizeof(args.error_message),
                    "Error: --spectator requires -ip <IP>");
            args.valid = false;
            return args;
        }

        args.mode = MODE_SPECTATOR;
        strncpy(args.ip_address, argv[3], sizeof(args.ip_address) - 1);

        // Check for optional -port argument
        if (argc >= 6 && strcmp(argv[4], "-port") == 0) {
            args.port = atoi(argv[5]);
            if (args.port <= 0 || args.port > 65535) {
                snprintf(args.error_message, sizeof(args.error_message),
                        "Error: Invalid port number '%s'. Port must be between 1 and 65535", argv[5]);
                args.valid = false;
                return args;
            }
        }
        return args;
    }

    // Unknown option
    snprintf(args.error_message, sizeof(args.error_message),
            "Error: Unknown option '%s'. Use --help for usage information", argv[1]);
    args.valid = false;
    return args;
}
