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

#include "config_util.h"

int
check_config_option(const cJSON *config_option, option_type_t type, const char *path)
{
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
