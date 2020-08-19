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

#ifndef _CONFIG_LOG_H_
#define _CONFIG_LOG_H_

#include <linux/limits.h>
#include "config_util.h"

/**
 * Logging options.
 */
typedef struct {
    char debug_log[PATH_MAX]; /**< The debug log file path */
    char error_log[PATH_MAX]; /**< The error log file path */
    int debug_mode; /**< Whether debug mode is on or not */
    int log_warnings; /**< Whether to log warnings in the error log or not */
} log_options_t;

log_options_t log_options;

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
 * Loads the log options from the configuration file.
 * @param[in] config The configuration object.
 * @return 0 on success or -1 on error.
 */
int
load_log_options(config_file_t *config);

#endif //_CONFIG_LOG_H_
