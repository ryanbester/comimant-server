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

#include "config_database.h"
#include "config_util.h"

int
load_database_options(config_file_t *config)
{
    if (NULL == config->config_json) {
        return -1;
    }

    // Initiate temporary variables
    const cJSON *database_obj = NULL;
    const cJSON *host = NULL;
    const cJSON *port = NULL;
    const cJSON *database = NULL;
    const cJSON *user = NULL;
    const cJSON *password = NULL;
    const cJSON *user_modify = NULL;
    const cJSON *password_modify = NULL;
    const cJSON *user_delete = NULL;
    const cJSON *password_delete = NULL;

    database_obj = cJSON_GetObjectItemCaseSensitive(config->config_json, "database");
    if (-1 == check_config_option(database_obj, OBJECT, "database")) {
        return -1;
    }

    host = cJSON_GetObjectItemCaseSensitive(database_obj, "host");
    if (-1 == check_config_option(host, STRING, "database.host")) {
        return -1;
    }
    strcpy(database_options.host, host->valuestring);

    port = cJSON_GetObjectItemCaseSensitive(database_obj, "port");
    if (-1 == check_config_option(port, NUMBER, "database.port")) {
        return -1;
    }
    database_options.port = port->valueint;

    database = cJSON_GetObjectItemCaseSensitive(database_obj, "database");
    if (-1 == check_config_option(database, STRING, "database.database")) {
        return -1;
    }
    strcpy(database_options.database, database->valuestring);

    user = cJSON_GetObjectItemCaseSensitive(database_obj, "user");
    if (-1 == check_config_option(user, STRING, "database.user")) {
        return -1;
    }
    strcpy(database_options.user, user->valuestring);

    password = cJSON_GetObjectItemCaseSensitive(database_obj, "password");
    if (-1 == check_config_option(password, STRING, "database.password")) {
        return -1;
    }
    strcpy(database_options.password, password->valuestring);

    user_modify = cJSON_GetObjectItemCaseSensitive(database_obj, "user_modify");
    if (-1 == check_config_option(user_modify, STRING, "database.user_modify")) {
        return -1;
    }
    strcpy(database_options.user_modify, user_modify->valuestring);

    password_modify = cJSON_GetObjectItemCaseSensitive(database_obj, "password_modify");
    if (-1 == check_config_option(password_modify, STRING, "database.password_modify")) {
        return -1;
    }
    strcpy(database_options.password_modify, password_modify->valuestring);

    user_delete = cJSON_GetObjectItemCaseSensitive(database_obj, "user_delete");
    if (-1 == check_config_option(user_delete, STRING, "database.user_delete")) {
        return -1;
    }
    strcpy(database_options.user_delete, user_delete->valuestring);

    password_delete = cJSON_GetObjectItemCaseSensitive(database_obj, "password_delete");
    if (-1 == check_config_option(password_delete, STRING, "database.password_delete")) {
        return -1;
    }
    strcpy(database_options.password_delete, password_delete->valuestring);

    return 0;
}
