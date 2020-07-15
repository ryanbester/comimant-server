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
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "config.h"
#include "include/cjson/cJSON.h"

int read_config(config_file_t *config_file) {
    FILE *fp;

    fp = fopen(config_file->file_name, "r");

    if (NULL == fp) {
        fprintf(stderr, "[Config] Failed to open config file: %s: %s\n", config_file->file_name, strerror(errno));
        return 0;
    }

    fseek(fp, 0L, SEEK_END);
    size_t file_size = ftell(fp);
    rewind(fp);

    config_file->data = calloc(1, file_size + 1);
    if (!config_file->data) {
        fclose(fp);
        fprintf(stderr, "[Config] Memory allocation failed");
        return 0;
    }

    if (1 != fread(config_file->data, file_size, 1, fp)) {
        fclose(fp);
        free(config_file->data);
        fprintf(stderr, "[Config] Failed to read config file: %s\n", config_file->file_name);
        return 0;
    }

    return 1;
}

int parse_config(config_file_t *config) {
    if (NULL == config->data) {
        return 0;
    }

    config->config_json = cJSON_Parse(config->data);
    if (NULL == config->config_json) {
        const char *err_ptr = cJSON_GetErrorPtr();
        if (NULL != err_ptr) {
            fprintf(stderr, "[Config] Malformed JSON before: %.*s\n", 25, err_ptr);
        }

        cJSON_Delete(config->config_json);
        return 0;
    }

    return 1;
}

int check_config_option(const cJSON *config_option, option_type_t type, const char *path) {
    if (NULL == config_option) {
        return 0;
    }

    switch (type) {
        case OBJECT:
            if (!cJSON_IsObject(config_option)) {
                fprintf(stderr, "[Config] %s is not an object", path);
                return 0;
            }
            break;
        case ARRAY:
            if (!cJSON_IsArray(config_option)) {
                fprintf(stderr, "[Config] %s is not an array", path);
                return 0;
            }
            break;
        case STRING:
            if (!cJSON_IsString(config_option)) {
                fprintf(stderr, "[Config] %s is not a string", path);
                return 0;
            }
            break;
        case BOOLEAN:
            if (!cJSON_IsBool(config_option)) {
                fprintf(stderr, "[Config] %s is not a boolean", path);
                return 0;
            }
            break;
        case NUMBER:
            if (!cJSON_IsNumber(config_option)) {
                fprintf(stderr, "[Config] %s is not a number", path);
                return 0;
            }
            break;
        case T_NULL:
            if (!cJSON_IsNull(config_option)) {
                fprintf(stderr, "[Config] %s is not null", path);
                return 0;
            }
            break;
    }

    return 1;
}

int load_listen_options(config_file_t *config, listen_options_t *listen_options) {
    if (NULL == config->config_json) {
        return 0;
    }

    // Set default options
    listen_options->lo_tcp_enabled = 0;
    listen_options->lo_tcp = &(listen_option_tcp_t) {
            DEFAULT_ADDRESS,
            DEFAULT_PORT,
            DEFAULT_BACKLOG
    };
    listen_options->lo_unix_enabled = 1;
    listen_options->lo_unix = &(listen_option_unix_t) {
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
    if (0 == check_config_option(listen_obj, OBJECT, "listen")) {
        return 0;
    }

    listen_tcp_obj = cJSON_GetObjectItemCaseSensitive(listen_obj, "tcp");
    if (NULL != listen_tcp_obj) {
        if (0 == check_config_option(listen_tcp_obj, OBJECT, "listen.tcp")) {
            return 0;
        }

        tcp_enabled = cJSON_GetObjectItemCaseSensitive(listen_tcp_obj, "enabled");
        if (NULL != tcp_enabled) {
            if (0 == check_config_option(tcp_enabled, BOOLEAN, "listen.tcp.enabled")) {
                return 0;
            }
            listen_options->lo_tcp_enabled = tcp_enabled->valueint;
        }

        listen_address = cJSON_GetObjectItemCaseSensitive(listen_tcp_obj, "address");
        if (NULL != listen_address) {
            if (0 == check_config_option(listen_address, STRING, "listen.tcp.address")) {
                return 0;
            }
            listen_options->lo_tcp->address = listen_address->valuestring;
        }

        listen_port = cJSON_GetObjectItemCaseSensitive(listen_tcp_obj, "port");
        if (NULL != listen_port) {
            if (0 == check_config_option(listen_port, NUMBER, "listen.tcp.port")) {
                return 0;
            }
            listen_options->lo_tcp->port = listen_port->valueint;
        }

        tcp_backlog = cJSON_GetObjectItemCaseSensitive(listen_tcp_obj, "backlog");
        if (NULL != tcp_backlog) {
            if (0 == check_config_option(tcp_backlog, NUMBER, "listen.tcp.backlog")) {
                return 0;
            }
            listen_options->lo_tcp->backlog = tcp_backlog->valueint;
        }
    }

    listen_unix_obj = cJSON_GetObjectItemCaseSensitive(listen_obj, "unix");
    if (NULL != listen_unix_obj) {
        if (0 == check_config_option(listen_unix_obj, OBJECT, "listen.unix")) {
            return 0;
        }

        unix_enabled = cJSON_GetObjectItemCaseSensitive(listen_unix_obj, "enabled");
        if (NULL != unix_enabled) {
            if (0 == check_config_option(unix_enabled, BOOLEAN, "listen.unix.enabled")) {
                return 0;
            }
            listen_options->lo_unix_enabled = unix_enabled->valueint;
        }

        sock_file = cJSON_GetObjectItemCaseSensitive(listen_unix_obj, "sock_file");
        if (NULL != sock_file) {
            if (0 == check_config_option(sock_file, STRING, "listen.unix.sock_file")) {
                return 0;
            }
            listen_options->lo_unix->sock_file = sock_file->valuestring;
        }

        unix_backlog = cJSON_GetObjectItemCaseSensitive(listen_unix_obj, "backlog");
        if (NULL != unix_backlog) {
            if (0 == check_config_option(unix_backlog, NUMBER, "listen.unix.backlog")) {
                return 0;
            }
            listen_options->lo_unix->backlog = unix_backlog->valueint;
        }
    }

    return 1;
}

void free_config(config_file_t *config) {
    free(config->data);
    cJSON_Delete(config->config_json);
}
