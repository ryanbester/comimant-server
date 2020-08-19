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

#include "config_plugins.h"
#include "config_util.h"

int
load_plugins_options(config_file_t *config)
{
    if (NULL == config->config_json) {
        return -1;
    }

    // Initiate temporary variables
    const cJSON *plugins_obj = NULL;

    plugins_obj = cJSON_GetObjectItemCaseSensitive(config->config_json, "plugins");
    if (NULL != plugins_obj) {
        if (-1 == check_config_option(plugins_obj, OBJECT, "plugins")) {
            return -1;
        }
    }

    return 0;
}
