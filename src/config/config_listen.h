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

#ifndef _CONFIG_LISTEN_H_
#define _CONFIG_LISTEN_H_

#include <linux/limits.h>
#include "config_util.h"

/**
 * TCP listen options
 */
typedef struct {
    char address[16]; /**< The IP address */
    int port; /**< The port */
    int backlog; /**< The backlog */
} listen_option_tcp_t;

/**
 * Unix socket listen options
 */
typedef struct {
    char sock_file[PATH_MAX]; /**< The path to the socket file */
    int backlog; /**< The backlog */
} listen_option_unix_t;

/**
 * Listen options structure
 */
typedef struct {
    int lo_tcp_enabled; /**< TCP Enabled */
    int lo_unix_enabled; /**< Unix Enabled */
    listen_option_tcp_t lo_tcp; /**< TCP options */
    listen_option_unix_t lo_unix; /**< Unix options */
} listen_options_t;

listen_options_t listen_options;


/**
 * Default TCP listening address
 */
#define DEFAULT_ADDRESS "127.0.0.1"

/**
 * Default TCP port
 */
#define DEFAULT_PORT 6481

/**
 * Default backlog
 */
#define DEFAULT_BACKLOG 10

/**
 * Default unix socket file path
 */
#define DEFAULT_SOCK_FILE "/var/run/comimant.sock"

/**
 * Loads the listen options from the configuration file.
 * @param[in] config The configuration object.
 * @return 0 on success or -1 on error.
 */
int
load_listen_options(config_file_t *config);

#endif //_CONFIG_LISTEN_H_
