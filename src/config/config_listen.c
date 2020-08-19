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

#include <string.h>

#include "config_listen.h"
#include "config_util.h"

int
load_listen_options(config_file_t *config)
{
    if (NULL == config->config_json) {
        return -1;
    }

    // Set default options
    listen_options.lo_tcp_enabled = 0;
    listen_options.lo_tcp = (listen_option_tcp_t) {
            DEFAULT_ADDRESS,
            DEFAULT_PORT,
            DEFAULT_BACKLOG
    };
    listen_options.lo_unix_enabled = 1;
    listen_options.lo_unix = (listen_option_unix_t) {
            DEFAULT_SOCK_FILE,
            DEFAULT_BACKLOG
    };

    // Initiate temporary variables
    const cJSON *listen_obj = NULL;
    const cJSON *listen_tcp_obj = NULL;
    const cJSON *listen_unix_obj = NULL;

    const cJSON *tcp_enabled = NULL;
    const cJSON *unix_enabled = NULL;
    const cJSON *listen_address = NULL;
    const cJSON *listen_port = NULL;
    const cJSON *tcp_backlog = NULL;
    const cJSON *sock_file = NULL;
    const cJSON *unix_backlog = NULL;

    listen_obj = cJSON_GetObjectItemCaseSensitive(config->config_json, "listen");
    if (-1 == check_config_option(listen_obj, OBJECT, "listen")) {
        return -1;
    }

    listen_tcp_obj = cJSON_GetObjectItemCaseSensitive(listen_obj, "tcp");
    if (NULL != listen_tcp_obj) {
        if (-1 == check_config_option(listen_tcp_obj, OBJECT, "listen.tcp")) {
            return -1;
        }

        tcp_enabled = cJSON_GetObjectItemCaseSensitive(listen_tcp_obj, "enabled");
        if (NULL != tcp_enabled) {
            if (-1 == check_config_option(tcp_enabled, BOOLEAN, "listen.tcp.enabled")) {
                return -1;
            }
            listen_options.lo_tcp_enabled = tcp_enabled->valueint;
        }

        listen_address = cJSON_GetObjectItemCaseSensitive(listen_tcp_obj, "address");
        if (NULL != listen_address) {
            if (-1 == check_config_option(listen_address, STRING, "listen.tcp.address")) {
                return -1;
            }
            strcpy(listen_options.lo_tcp.address, listen_address->valuestring);
        }

        listen_port = cJSON_GetObjectItemCaseSensitive(listen_tcp_obj, "port");
        if (NULL != listen_port) {
            if (-1 == check_config_option(listen_port, NUMBER, "listen.tcp.port")) {
                return -1;
            }
            listen_options.lo_tcp.port = listen_port->valueint;
        }

        tcp_backlog = cJSON_GetObjectItemCaseSensitive(listen_tcp_obj, "backlog");
        if (NULL != tcp_backlog) {
            if (-1 == check_config_option(tcp_backlog, NUMBER, "listen.tcp.backlog")) {
                return -1;
            }
            listen_options.lo_tcp.backlog = tcp_backlog->valueint;
        }
    }

    listen_unix_obj = cJSON_GetObjectItemCaseSensitive(listen_obj, "unix");
    if (NULL != listen_unix_obj) {
        if (-1 == check_config_option(listen_unix_obj, OBJECT, "listen.unix")) {
            return -1;
        }

        unix_enabled = cJSON_GetObjectItemCaseSensitive(listen_unix_obj, "enabled");
        if (NULL != unix_enabled) {
            if (-1 == check_config_option(unix_enabled, BOOLEAN, "listen.unix.enabled")) {
                return -1;
            }
            listen_options.lo_unix_enabled = unix_enabled->valueint;
        }

        sock_file = cJSON_GetObjectItemCaseSensitive(listen_unix_obj, "sock_file");
        if (NULL != sock_file) {
            if (-1 == check_config_option(sock_file, STRING, "listen.unix.sock_file")) {
                return -1;
            }
            strcpy(listen_options.lo_unix.sock_file, sock_file->valuestring);
        }

        unix_backlog = cJSON_GetObjectItemCaseSensitive(listen_unix_obj, "backlog");
        if (NULL != unix_backlog) {
            if (-1 == check_config_option(unix_backlog, NUMBER, "listen.unix.backlog")) {
                return -1;
            }
            listen_options.lo_unix.backlog = unix_backlog->valueint;
        }
    }

    return 0;
}
