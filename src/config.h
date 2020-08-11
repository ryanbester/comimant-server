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

/**
 * @file
 * @brief Config.h
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <limits.h>
#include "../include/cjson/cJSON.h"

/**
 * The data type of the configuration option.
 */
typedef enum {
    OBJECT, /**< An object */
    ARRAY, /**< An array */
    STRING, /**< A string */
    BOOLEAN, /**< A boolean */
    NUMBER, /**< An integer or double */
    T_NULL /**< Null */
} option_type_t;

/**
 * Represents a configuration file
 */
typedef struct {
    const char *file_name; /**< The file name */
    char *data; /**< The raw data */
    cJSON *config_json; /**< The parsed JSON object */
} config_file_t;

/**
 * Logging options.
 */
typedef struct {
    char debug_log[PATH_MAX]; /**< The debug log file path */
    char error_log[PATH_MAX]; /**< The error log file path */
    int debug_mode; /**< Whether debug mode is on or not */
    int log_warnings; /**< Whether to log warnings in the error log or not */
} log_options_t;

/**
 * SSL options
 */
typedef struct {
    char cert_path[PATH_MAX]; /**< The SSL certificate path */
    char key_path[PATH_MAX]; /**< The SSL private key path */
} ssl_options_t;

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

/**
 * Default debug log path
 */
#define DEFAULT_DEBUG_LOG "/var/log/comimant/debug.log"

/**
 * Default error log path
 */
#define DEFAULT_ERROR_LOG "/var/log/comimant/error.log"

/**
 * Default debug mode setting
 */
#define DEFAULT_DEBUG_MODE 0

/**
 * Default log warnings setting
 */
#define DEFAULT_LOG_WARNINGS 0

/**
 * Default SSL certificate path
 */
#define DEFAULT_CERT_PATH "cert.pem"

/**
 * Default SSL private key path
 */
#define DEFAULT_KEY_PATH "key.pem"

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
 * Default PID file path
 */
#define DEFAULT_PID_FILE "/var/run/comimant.pid"

/**
 * Reads the raw data from the configuration file. The @ref config_file_t::data "data" field of the configuration file structure will be set
 * accordingly.
 * @param[in,out] config The configuration file structure.
 * @return 0 on success or -1 on error.
 */
int
read_config(config_file_t *config);

/**
 * Parses the raw data from the configuration file. The @ref config_file_t::data "data" field of the configuration file structure must be set.
 * The @ref config_file_t::config_json "config_json" field will contain the parsed JSON object.
 * @param[in,out] config The configuration file structure.
 * @return 0 on success or -1 on error.
 */
int
parse_config(config_file_t *config);

/**
 * Checks if a config option is of a certain data type and is set.
 * @param[in] config_option The configuration option.
 * @param[in] type The data type.
 * @param[in] path The path of the configuration option to print in error messages.
 * @return 0 if it is valid or -1 if it is invalid.
 */
int
check_config_option(const cJSON *config_option, option_type_t type, const char *path);

/**
 * Loads the log options from the configuration file.
 * @param[in] config The configuration object.
 * @param[out] log_options An empty @ref log_options_t structure.
 * @return 0 on success or -1 on error.
 */
int
load_log_options(config_file_t *config, log_options_t *log_options);

/**
 * Loads the SSL options from the configuration file.
 * @param[in] config The configuration object.
 * @param[out] ssl_options An empty @ref ssl_options_t structure.
 * @return 0 on success or -1 on error.
 */
int
load_ssl_options(config_file_t *config, ssl_options_t *ssl_options);

/**
 * Loads the listen options from the configuration file.
 * @param[in] config The configuration object.
 * @param[out] listen_options An empty @ref listen_options_t structure.
 * @return 0 on success or -1 on error.
 */
int
load_listen_options(config_file_t *config, listen_options_t *listen_options);

/**
 * Gets the PID file path from the configuration file.
 * @param[in] config The configuration object.
 * @param[out] buffer A buffer to store the file path. This must have capacity for the path.
 * @return 0 on success or -1 on error.
 */
int
get_pid_file(config_file_t *config, char *buffer);

/**
 * Frees the allocated memory.
 * @param[in] config The configuration object.
 */
void
free_config(config_file_t *config);

#endif //_CONFIG_H_
