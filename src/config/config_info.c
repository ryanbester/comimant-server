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
#include <stdio.h>

#include "config_info.h"
#include "config_util.h"

int
load_info(config_file_t *config)
{
    if (NULL == config->config_json) {
        return -1;
    }

    // Initiate temporary variables
    const cJSON *info_obj = NULL;
    const cJSON *server_name = NULL;

    info_obj = cJSON_GetObjectItemCaseSensitive(config->config_json, "info");
    if (-1 == check_config_option(info_obj, OBJECT, "info")) {
        return -1;
    }

    server_name = cJSON_GetObjectItemCaseSensitive(info_obj, "server_name");
    if (-1 == check_config_option(server_name, STRING, "info.server_name")) {
        return -1;
    }
    strcpy(config_info.server_name, server_name->valuestring);

    return 0;
}
