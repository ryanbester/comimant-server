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

#include "config/config_log.h"

#include "../include/log/log.h"

#include "plugins/plugins.h"

static int verbose_flag;
static char pid_file[PATH_MAX] = DEFAULT_PID_FILE;

void
show_opt_err();

void
show_help();

int
init_plugins(const char *plugins_dir);

int
load_config_options(const char *file_name);

int
main(int argc, char **argv)
{
    char config_file[PATH_MAX] = "/etc/comimant-server/config.json";
    char plugins_dir[PATH_MAX] = "/etc/comimant-server/plugins";

    while (1) {
        static struct option long_options[] = {
                {"help",        no_argument,       0,             0},
                {"verbose",     no_argument,       &verbose_flag, 1},
                {"config",      required_argument, 0,             'f'},
                {"plugins-dir", required_argument, 0,             'p'},
                {0, 0,                             0,             0},
        };

        int option_index = 0;

        int c = getopt_long(argc, argv, "vf:p:", long_options, &option_index);
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
                    deactivate_plugins();
                    return 0;
                }

                if (!strcmp(long_options[option_index].name, "config")) {
                    strcpy(config_file, optarg);
                }

                if (!strcmp(long_options[option_index].name, "plugins-dir")) {
                    strcpy(plugins_dir, optarg);
                }

                break;
            case 'f':
                strcpy(config_file, optarg);
                break;
            case 'p':
                strcpy(plugins_dir, optarg);
                break;
            case 'v':
                verbose_flag = 1;
                break;
            case '?':
                show_opt_err();
                deactivate_plugins();
                return 1;
            default:
                show_opt_err();
                deactivate_plugins();
                return 1;
        }
    }

    if (-1 == init_plugins(plugins_dir)) {
        return 1;
    }

    if (-1 == load_config_options(config_file)) {
        deactivate_plugins();
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
    deactivate_plugins();
    free_plugins();
    return 0;
}

void
show_opt_err()
{
    printf("Try 'comimant-server --help' for more information.\n");
}

void
show_help()
{
    printf("Usage: comimant-server [OPTION]...\n");
}

int
init_plugins(const char *plugins_dir)
{
    if (-1 == load_plugins_dir(plugins_dir)) {
        return -1;
    }

    if (-1 == load_plugins()) {
        return -1;
    }

    if (-1 == register_plugins()) {
        return -1;
    }

    if (-1 == activate_plugins()) {
        return -1;
    }

    return 0;
}

int
load_config_options(const char *file_name)
{
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

    init_default_config_sections();
    init_plugin_config_sections();

    load_config_sections(&config);

    if (-1 == init_log()) {
        fprintf(stderr, "Cannot start Comimant Server: Error initiating log.\n");
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
