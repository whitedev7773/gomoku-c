#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chat.h"
#include "config.h"
#include "game.h"
#include "network.h"
#include "tui.h"
#include "utils.h"

typedef struct {
    tui_context_t *tui;
} app_state_t;

static void network_event_handler(const network_event_t *event, void *userdata) {
    app_state_t *state = (app_state_t *)userdata;
    if (!state || !state->tui || !event) {
        return;
    }
    tui_notify_network_event(state->tui, event);
}

static int read_int_with_default(const char *prompt, int default_value) {
    char buffer[32];
    printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return default_value;
    }
    if (buffer[0] == '\n') {
        return default_value;
    }
    return atoi(buffer);
}

static void read_string_with_default(const char *prompt, char *output, size_t output_len, const char *default_value) {
    if (!output || output_len == 0) {
        return;
    }
    char buffer[256];
    printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin) || buffer[0] == '\n') {
        snprintf(output, output_len, "%s", default_value ? default_value : "");
    } else {
        buffer[strcspn(buffer, "\r\n")] = '\0';
        snprintf(output, output_len, "%s", buffer);
    }
}

int main(void) {
    config_t config;
    if (config_load(&config, CONFIG_DEFAULT_FILE) < 0) {
        config.default_ai_level = AI_LEVEL_MEDIUM;
        config.turn_time_seconds = 30;
        config.default_port = 4242;
        snprintf(config.nickname, sizeof(config.nickname), "Player");
    }

    printf("=== Gomoku TUI ===\n");
    printf("Nickname: %s\n", config.nickname);
    printf("Select mode:\n");
    printf("  1) Single player vs AI\n");
    printf("  2) Host LAN game\n");
    printf("  3) Join LAN game\n");
    int mode_choice = read_int_with_default("Choice [1]: ", 1);

    ai_level_t ai_level = config.default_ai_level;
    uint16_t port = config.default_port;
    char host[128] = "127.0.0.1";

    if (mode_choice == 1) {
        printf("\nSelect AI difficulty:\n");
        printf("  1) Easy\n  2) Medium\n  3) Hard\n");
        int diff_choice = read_int_with_default("Choice [2]: ", (int)config.default_ai_level + 1);
        switch (diff_choice) {
            case 1:
                ai_level = AI_LEVEL_EASY;
                break;
            case 3:
                ai_level = AI_LEVEL_HARD;
                break;
            default:
                ai_level = AI_LEVEL_MEDIUM;
                break;
        }
    } else if (mode_choice == 2) {
        port = (uint16_t)read_int_with_default("Port [4242]: ", config.default_port);
    } else if (mode_choice == 3) {
        read_string_with_default("Server address [127.0.0.1]: ", host, sizeof(host), "127.0.0.1");
        port = (uint16_t)read_int_with_default("Port [4242]: ", config.default_port);
    } else {
        printf("Invalid choice. Exiting.\n");
        return 0;
    }

    chat_log_t chat_log;
    chat_log_init(&chat_log);

    game_t game;
    if (mode_choice == 1) {
        game_init_single(&game, &config, ai_level);
    } else if (mode_choice == 2) {
        game_init_multi_host(&game, &config);
    } else {
        game_init_multi_client(&game, &config);
    }
    game_start(&game);

    tui_context_t *tui = tui_create(&game, &chat_log);
    if (!tui) {
        fprintf(stderr, "Failed to initialize TUI\n");
        return 1;
    }

    app_state_t app_state = {0};
    app_state.tui = tui;

    network_session_t *session = NULL;
    if (mode_choice == 2) {
        if (network_start_server(&session, port, config.nickname, network_event_handler, &app_state) != 0) {
            tui_destroy(tui);
            fprintf(stderr, "Failed to start server on port %u\n", port);
            return 1;
        }
        chat_log_add(&chat_log, "System", "Waiting for an opponent to connect...");
    } else if (mode_choice == 3) {
        char message[128];
        snprintf(message, sizeof(message), "Connecting to %s:%u ...", host, port);
        chat_log_add(&chat_log, "System", message);
        if (network_connect(&session, host, port, config.nickname, network_event_handler, &app_state) != 0) {
            tui_destroy(tui);
            fprintf(stderr, "Failed to connect to %s:%u\n", host, port);
            return 1;
        }
    }

    tui_run(tui, session);

    if (session) {
        network_close(session);
    }

    tui_destroy(tui);
    return 0;
}
