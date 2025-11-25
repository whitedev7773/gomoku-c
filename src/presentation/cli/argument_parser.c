// src/presentation/cli/argument_parser.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "argument_parser.h"

void parse_arguments(int argc, char *argv[], AppConfig *cfg)
{
    memset(cfg, 0, sizeof(AppConfig));
    cfg->port = 0;

    int opt;
    int long_index = 0;

    static struct option long_opts[] = {
        {"force-small-shell", no_argument, 0, 1},
        {"singleplayer", no_argument, 0, 2},
        {"host", no_argument, 0, 3},
        {"join", no_argument, 0, 4},
        {"replay", required_argument, 0, 5},
        {"ip", required_argument, 0, 'i'},
        {"port", required_argument, 0, 'p'},
        {0, 0, 0, 0}};

    while (1)
    {
        opt = getopt_long(argc, argv, "i:p:", long_opts, &long_index);
        if (opt == -1)
            break;

        switch (opt)
        {
        case 1:
            cfg->force_small_shell = 1;
            break;
        case 2:
            cfg->singleplayer = 1;
            break;
        case 3:
            cfg->host = 1;
            break;
        case 4:
            cfg->join = 1;
            break;
        case 5:
            cfg->replay_mode = 1;
            strncpy(cfg->replay_path, optarg, sizeof(cfg->replay_path) - 1);
            break;

        case 'i':
            strncpy(cfg->ip, optarg, sizeof(cfg->ip) - 1);
            break;

        case 'p':
            cfg->port = atoi(optarg);
            break;

        default:
            fprintf(stderr, "Unknown argument\n");
            exit(1);
        }
    }
}
