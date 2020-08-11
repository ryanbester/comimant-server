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
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "config.h"
#include "../include/cjson/cJSON.h"

int
read_config(config_file_t *config) {
    FILE *fp = fopen(config->file_name, "r");

    if (NULL == fp) {
        fprintf(stderr, "[Config] Failed to open config file: %s: %s\n", config->file_name, strerror(errno));
        return 0;
    }

    // Get file size
    fseek(fp, 0L, SEEK_END);
    size_t file_size = ftell(fp);
    rewind(fp);

    config->data = calloc(1, file_size + 1);
    if (!config->data) {
        fclose(fp);
        fprintf(stderr, "[Config] Memory allocation failed\n");
        return -1;
    }

    if (1 != fread(config->data, file_size, 1, fp)) {
        fclose(fp);
        free(config->data);
        fprintf(stderr, "[Config] Failed to read config file: %s\n", config->file_name);
        return -1;
    }

    return 0;
}

int
parse_config(config_file_t *config) {
    if (NULL == config->data) {
        return -1;
    }

    config->config_json = cJSON_Parse(config->data);
    if (NULL == config->config_json) {
        const char *err_ptr = cJSON_GetErrorPtr();
        if (NULL != err_ptr) {
            fprintf(stderr, "[Config] Malformed JSON before: %.*s\n", 25, err_ptr);
        }

        cJSON_Delete(config->config_json);
        return -1;
    }

    return 0;
}

int
check_config_option(const cJSON *config_option, option_type_t type, const char *path) {
    if (NULL == config_option) {
        return -1;
    }

    switch (type) {
        case OBJECT:
            if (!cJSON_IsObject(config_option)) {
                fprintf(stderr, "[Config] %s is not an object\n", path);
                return -1;
            }
            break;
        case ARRAY:
            if (!cJSON_IsArray(config_option)) {
                fprintf(stderr, "[Config] %s is not an array\n", path);
                return -1;
            }
            break;
        case STRING:
            if (!cJSON_IsString(config_option)) {
                fprintf(stderr, "[Config] %s is not a string\n", path);
                return -1;
            }
            break;
        case BOOLEAN:
            if (!cJSON_IsBool(config_option)) {
                fprintf(stderr, "[Config] %s is not a boolean\n", path);
                return -1;
            }
            break;
        case NUMBER:
            if (!cJSON_IsNumber(config_option)) {
                fprintf(stderr, "[Config] %s is not a number\n", path);
                return -1;
            }
            break;
        case T_NULL:
            if (!cJSON_IsNull(config_option)) {
                fprintf(stderr, "[Config] %s is not null\n", path);
                return -1;
            }
            break;
    }

    return 0;
}

int
load_log_options(config_file_t *config, log_options_t *log_options) {
    if (NULL == config->config_json) {
        return -1;
    }

    // Set default options
    strcpy(log_options->debug_log, DEFAULT_DEBUG_LOG);
    strcpy(log_options->error_log, DEFAULT_ERROR_LOG);
    log_options->debug_mode = DEFAULT_DEBUG_MODE;
    log_options->log_warnings = DEFAULT_LOG_WARNINGS;

    // Initiate temporary variables
    const cJSON *log_obj = NULL;
    const cJSON *debug_log = NULL;
    const cJSON *error_log = NULL;
    const cJSON *debug_mode = NULL;
    const cJSON *log_warnings = NULL;

    log_obj = cJSON_GetObjectItemCaseSensitive(config->config_json, "log");
    if (NULL != log_obj) {
        if (-1 == check_config_option(log_obj, OBJECT, "log")) {
            return -1;
        }

        debug_log = cJSON_GetObjectItemCaseSensitive(log_obj, "debug_log");
        if (NULL != debug_log) {
            if (-1 == check_config_option(debug_log, STRING, "log.debug_log")) {
                return -1;
            }
            strcpy(log_options->debug_log, debug_log->valuestring);
        }

        error_log = cJSON_GetObjectItemCaseSensitive(log_obj, "error_log");
        if (NULL != error_log) {
            if (-1 == check_config_option(error_log, STRING, "log.error_log")) {
                return -1;
            }
            strcpy(log_options->error_log, error_log->valuestring);
        }

        debug_mode = cJSON_GetObjectItemCaseSensitive(log_obj, "debug_mode");
        if (NULL != debug_mode) {
            if (-1 == check_config_option(debug_mode, BOOLEAN, "log.debug_mode")) {
                return -1;
            }
            log_options->debug_mode = debug_mode->valueint;
        }

        log_warnings = cJSON_GetObjectItemCaseSensitive(log_obj, "log_warnings");
        if (NULL != log_warnings) {
            if (-1 == check_config_option(log_warnings, BOOLEAN, "log.log_warnings")) {
                return -1;
            }
            log_options->log_warnings = log_warnings->valueint;
        }
    }

    return 0;
}

int
load_ssl_options(config_file_t *config, ssl_options_t *ssl_options) {
    if (NULL == config->config_json) {
        return -1;
    }

    // Set default options
    strcpy(ssl_options->cert_path, DEFAULT_CERT_PATH);
    strcpy(ssl_options->key_path, DEFAULT_KEY_PATH);

    // Initiate temporary variables
    const cJSON *ssl_obj = NULL;
    const cJSON *cert_path = NULL;
    const cJSON *key_path = NULL;

    ssl_obj = cJSON_GetObjectItemCaseSensitive(config->config_json, "ssl");
    if (NULL != ssl_obj) {
        if (-1 == check_config_option(ssl_obj, OBJECT, "ssl")) {
            return -1;
        }

        cert_path = cJSON_GetObjectItemCaseSensitive(ssl_obj, "cert");
        if (NULL != cert_path) {
            if (-1 == check_config_option(cert_path, STRING, "ssl.cert")) {
                return -1;
            }
            strcpy(ssl_options->cert_path, cert_path->valuestring);
        }

        key_path = cJSON_GetObjectItemCaseSensitive(ssl_obj, "key");
        if (NULL != key_path) {
            if (-1 == check_config_option(key_path, STRING, "ssl.key")) {
                return -1;
            }
            strcpy(ssl_options->key_path, key_path->valuestring);
        }
    }

    return 0;
}

int
load_listen_options(config_file_t *config, listen_options_t *listen_options) {
    if (NULL == config->config_json) {
        return -1;
    }

    // Set default options
    listen_options->lo_tcp_enabled = 0;
    listen_options->lo_tcp = (listen_option_tcp_t) {
            DEFAULT_ADDRESS,
            DEFAULT_PORT,
            DEFAULT_BACKLOG
    };
    listen_options->lo_unix_enabled = 1;
    listen_options->lo_unix = (listen_option_unix_t) {
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
            listen_options->lo_tcp_enabled = tcp_enabled->valueint;
        }

        listen_address = cJSON_GetObjectItemCaseSensitive(listen_tcp_obj, "address");
        if (NULL != listen_address) {
            if (-1 == check_config_option(listen_address, STRING, "listen.tcp.address")) {
                return -1;
            }
            strcpy(listen_options->lo_tcp.address, listen_address->valuestring);
        }

        listen_port = cJSON_GetObjectItemCaseSensitive(listen_tcp_obj, "port");
        if (NULL != listen_port) {
            if (-1 == check_config_option(listen_port, NUMBER, "listen.tcp.port")) {
                return -1;
            }
            listen_options->lo_tcp.port = listen_port->valueint;
        }

        tcp_backlog = cJSON_GetObjectItemCaseSensitive(listen_tcp_obj, "backlog");
        if (NULL != tcp_backlog) {
            if (-1 == check_config_option(tcp_backlog, NUMBER, "listen.tcp.backlog")) {
                return -1;
            }
            listen_options->lo_tcp.backlog = tcp_backlog->valueint;
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
            listen_options->lo_unix_enabled = unix_enabled->valueint;
        }

        sock_file = cJSON_GetObjectItemCaseSensitive(listen_unix_obj, "sock_file");
        if (NULL != sock_file) {
            if (-1 == check_config_option(sock_file, STRING, "listen.unix.sock_file")) {
                return -1;
            }
            strcpy(listen_options->lo_unix.sock_file, sock_file->valuestring);
        }

        unix_backlog = cJSON_GetObjectItemCaseSensitive(listen_unix_obj, "backlog");
        if (NULL != unix_backlog) {
            if (-1 == check_config_option(unix_backlog, NUMBER, "listen.unix.backlog")) {
                return -1;
            }
            listen_options->lo_unix.backlog = unix_backlog->valueint;
        }
    }

    return 0;
}

int
get_pid_file(config_file_t *config, char *pid_file) {
    if (NULL == config->config_json) {
        return -1;
    }

    // Initiate temporary variables
    const cJSON *pid_file_obj = NULL;

    pid_file_obj = cJSON_GetObjectItemCaseSensitive(config->config_json, "pid_file");
    if (NULL != pid_file_obj) {
        if (-1 == check_config_option(pid_file_obj, STRING, "pid_file")) {
            return -1;
        }
        strcpy(pid_file, pid_file_obj->valuestring);
    }

    return 0;
}

void
free_config(config_file_t *config) {
    free(config->data);
    cJSON_Delete(config->config_json);
}
