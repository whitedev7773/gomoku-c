// src/application/app_config.h
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

typedef struct
{
    int force_small_shell;

    int singleplayer;
    int host;
    int join;
    int replay_mode;

    char ip[32];
    int port;

    char replay_path[256];
} AppConfig;

#endif
