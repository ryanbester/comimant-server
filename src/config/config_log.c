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

#include "config_log.h"
#include "config_util.h"

int
load_log_options(config_file_t *config)
{
    if (NULL == config->config_json) {
        return -1;
    }

    // Set default options
    strcpy(log_options.debug_log, DEFAULT_DEBUG_LOG);
    strcpy(log_options.error_log, DEFAULT_ERROR_LOG);
    log_options.debug_mode = DEFAULT_DEBUG_MODE;
    log_options.log_warnings = DEFAULT_LOG_WARNINGS;

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
            strcpy(log_options.debug_log, debug_log->valuestring);
        }

        error_log = cJSON_GetObjectItemCaseSensitive(log_obj, "error_log");
        if (NULL != error_log) {
            if (-1 == check_config_option(error_log, STRING, "log.error_log")) {
                return -1;
            }
            strcpy(log_options.error_log, error_log->valuestring);
        }

        debug_mode = cJSON_GetObjectItemCaseSensitive(log_obj, "debug_mode");
        if (NULL != debug_mode) {
            if (-1 == check_config_option(debug_mode, BOOLEAN, "log.debug_mode")) {
                return -1;
            }
            log_options.debug_mode = debug_mode->valueint;
        }

        log_warnings = cJSON_GetObjectItemCaseSensitive(log_obj, "log_warnings");
        if (NULL != log_warnings) {
            if (-1 == check_config_option(log_warnings, BOOLEAN, "log.log_warnings")) {
                return -1;
            }
            log_options.log_warnings = log_warnings->valueint;
        }
    }

    return 0;
}
