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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "../include/cjson/cJSON.h"

typedef enum {
    OBJECT,
    ARRAY,
    STRING,
    BOOLEAN,
    NUMBER,
    T_NULL
} option_type_t;

typedef struct {
    const char *file_name;
    char *data;
    cJSON *config_json;
} config_file_t;

typedef struct {
    char *address;
    int port;
    int backlog;
} listen_option_tcp_t;

typedef struct {
    char *sock_file;
    int backlog;
} listen_option_unix_t;

typedef struct {
    int lo_tcp_enabled;
    int lo_unix_enabled;
    listen_option_tcp_t *lo_tcp;
    listen_option_unix_t *lo_unix;
} listen_options_t;

#define DEFAULT_ADDRESS "127.0.0.1"
#define DEFAULT_PORT 6481
#define DEFAULT_BACKLOG 10
#define DEFAULT_SOCK_FILE "/var/run/comimant.sock"

#define DEFAULT_PID_FILE "/var/run/comimant.pid"

int read_config(config_file_t *config_file);

int parse_config(config_file_t *config);

int check_config_option(const cJSON *config_option, option_type_t type, const char *path);

int load_listen_options(config_file_t *config, listen_options_t *listen_options);

void free_config(config_file_t *config);

#endif //_CONFIG_H_
