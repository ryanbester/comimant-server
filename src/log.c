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

#include <stdarg.h>

#include "log.h"
#include "../include/log/log.h"

int
init_log() {
    // Grab a file pointer to the debug log
    FILE *debug_log_fp = fopen(log_options.debug_log, "a+");

    if (NULL == debug_log_fp) {
        fprintf(stderr, "[Log] Failed to open debug log file: %s\n", log_options.debug_log);
        return -1;
    }

    // If debug mode is on, enable more verbosity
    if (1 == log_options.debug_mode) {
        if (log_add_fp(debug_log_fp, LOG_TRACE) < 0) {
            fprintf(stderr, "[Log] Failed to register debug log\n");
            return -1;
        }
    } else {
        if (log_add_fp(debug_log_fp, LOG_INFO) < 0) {
            fprintf(stderr, "[Log] Failed to register debug log\n");
            return -1;
        }
    }

    // Do the same for the error log
    FILE *error_log_fp = fopen(log_options.error_log, "a+");

    if (NULL == error_log_fp) {
        fprintf(stderr, "[Log] Failed to open error log file: %s\n", log_options.error_log);
        return -1;
    }

    // If log warnings is on, allow warnings to be logged in the error log.
    if (1 == log_options.log_warnings) {
        if (log_add_fp(error_log_fp, LOG_WARN) < 0) {
            fprintf(stderr, "[Log] Failed to register error log\n");
            return -1;
        }
    } else {
        if (log_add_fp(error_log_fp, LOG_ERROR) < 0) {
            fprintf(stderr, "[Log] Failed to register error log\n");
            return -1;
        }
    }

    // Turn off logging to stderr.
    log_set_quiet(true);

    return 0;
}

