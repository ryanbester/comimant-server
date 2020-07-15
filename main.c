/**
 * Comimant Server
 * Copyright (C) 2019 - 2020 Ryan Bester
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <getopt.h>
#include <stdlib.h>

#include "main.h"
#include "config.h"

static int verbose_flag;

void show_opt_err();

void show_help();

int main(int argc, char **argv) {
    openlog(APP_NAME, LOG_CONS | LOG_PID, LOG_LOCAL1);

    char config_file[PATH_MAX] = "/etc/comimant-server/config.json";

    while (1) {
        static struct option long_options[] = {
                {"help",    no_argument,       0,             0},
                {"verbose", no_argument,       &verbose_flag, 1},
                {"config",  required_argument, 0,             'f'},
                {0, 0,                         0,             0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "vf:", long_options, &option_index);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 0:
                if (long_options[option_index].flag != 0) {
                    break;
                }

                if (!strcmp(long_options[option_index].name, "help")) {
                    show_help();
                    closelog();
                    return 0;
                }

                if (!strcmp(long_options[option_index].name, "config")) {
                    strcpy(config_file, optarg);
                }

                break;
            case 'f':
                strcpy(config_file, optarg);
                break;
            case 'v':
                verbose_flag = 1;
                break;
            case '?':
                show_opt_err();
                closelog();
                return 1;
            default:
                show_opt_err();
                closelog();
                return 1;
        }
    }

    config_file_t config;
    memset(&config, 0, sizeof(config));
    config.file_name = config_file;
    read_config(&config);

    parse_config(&config);

    listen_options_t listen_options;
    memset(&listen_options, 0, sizeof(listen_options));
    load_listen_options(&config, &listen_options);

    free_config(&config);

    closelog();
    return 0;
}

void show_opt_err() {
    printf("Try 'comimant-server --help' for more information.\n");
}

void show_help() {
    printf("Usage: comimant-server [OPTION]...\n");
}
