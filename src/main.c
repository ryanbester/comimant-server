/*
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
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <getopt.h>
#include <stdlib.h>
#include <signal.h>

#include "main.h"
#include "log.h"
#include "daemon.h"

#include "../include/log/log.h"

static int verbose_flag;
static char pid_file[PATH_MAX] = DEFAULT_PID_FILE;

void
show_opt_err();

void
show_help();

int
load_config_options(const char *file_name);

int
main(int argc, char **argv) {
    char config_file[PATH_MAX] = "/etc/comimant-server/config.json";\

    while (1) {
        static struct option long_options[] = {
                {"help",    no_argument,       0,             0},
                {"verbose", no_argument,       &verbose_flag, 1},
                {"config",  required_argument, 0,             'f'},
                {0, 0,                         0,             0},
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
                return 1;
            default:
                show_opt_err();
                return 1;
        }
    }

    if (-1 == load_config_options(config_file)) {
        return 1;
    }

    int server_running = 0;
    int running_pid = get_pid(pid_file);

    if (running_pid <= 0) {
        log_info("Server is not running");
    } else {
        server_running = 1;
        log_info("Server already running on PID: %d", running_pid);
    }

    if (NULL != argv[optind]) {
        if (!strcmp(argv[optind], "status")) {
            if (1 == server_running) {
                printf("Server running on PID: %d\n", running_pid);
            } else {
                printf("Server not running\n");
            }
            goto exit;
        } else if (!strcmp(argv[optind], "stop")) {
            if (1 == server_running) {
                printf("Stopping server...\n");
                log_info("Stopping server...");

                daemon_t daemon = {
                        running_pid
                };
                daemon_stop_server(&daemon, pid_file);
                printf("Stopped server\n");
            } else {
                printf("Server not running\n");
            }
            goto exit;
        } else if (!strcmp(argv[optind], "force-stop")) {
            if (1 == server_running) {
                printf("Stopping server forcefully...\n");
                log_info("Stopping server forcefully...");

                daemon_t daemon = {
                        running_pid
                };
                daemon_force_stop_server(&daemon, pid_file);
                printf("Stopped server\n");
            } else {
                printf("Server not running\n");
            }
            goto exit;
        } else {
            printf("Unknown command\n");
            show_opt_err();
            goto exit;
        }
    } else {
        if (1 == server_running) {
            printf("Server already running. Not starting again.\n");
            goto exit;
        }
    }

    log_info("Server startup initiated by user: %d", getuid());
    log_info("Starting server...");

    if (-1 == daemon_start_server(pid_file)) {
        fprintf(stderr, "Error starting server\n");
    }

    exit:
    return 0;
}

void
show_opt_err() {
    printf("Try 'comimant-server --help' for more information.\n");
}

void
show_help() {
    printf("Usage: comimant-server [OPTION]...\n");
}

int
load_config_options(const char *file_name) {
    config_file_t config;
    memset(&config, 0, sizeof(config));
    config.file_name = file_name;

    if (-1 == read_config(&config)) {
        fprintf(stderr, "Cannot start Comimant Server: Error reading configuration file.\n");
        goto fail;
    }

    if (-1 == parse_config(&config)) {
        fprintf(stderr, "Cannot start Comimant Server: Error parsing configuration file.\n");
        goto fail;
    }

    memset(&log_options, 0, sizeof(log_options));
    if (-1 == load_log_options(&config, &log_options)) {
        fprintf(stderr, "Cannot start Comimant Server: Error loading log options.\n");
        goto fail;
    }

    if (-1 == init_log()) {
        fprintf(stderr, "Cannot start Comimant Server: Error initiating log.\n");
        goto fail;
    }

    memset(&ssl_options, 0, sizeof(ssl_options));
    if (-1 == load_ssl_options(&config, &ssl_options)) {
        fprintf(stderr, "Cannot start Comimant Server: Error loading SSL options.\n");
        goto fail;
    }

    memset(&listen_options, 0, sizeof(listen_options));

    if (-1 == load_listen_options(&config, &listen_options)) {
        fprintf(stderr, "Cannot start Comimant Server: Error loading listen options.\n");
        goto fail;
    }

    if (-1 == get_pid_file(&config, pid_file)) {
        fprintf(stderr, "Cannot start Comimant Server: Error loading PID file path.\n");
        goto fail;
    }

    free_config(&config);
    return 0;

    fail:
    free_config(&config);
    return -1;
}
