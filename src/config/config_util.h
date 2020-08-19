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

#ifndef _CONFIG_UTIL_H_
#define _CONFIG_UTIL_H_

#include "../../include/cjson/cJSON.h"

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
 * Checks if a config option is of a certain data type and is set.
 * @param[in] config_option The configuration option.
 * @param[in] type The data type.
 * @param[in] path The path of the configuration option to print in error messages.
 * @return 0 if it is valid or -1 if it is invalid.
 */
int
check_config_option(const cJSON *config_option, option_type_t type, const char *path);

#endif //_CONFIG_UTIL_H_
