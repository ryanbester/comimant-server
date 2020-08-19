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

#include "config_ssl.h"
#include "config_util.h"

int
load_ssl_options(config_file_t *config)
{
    if (NULL == config->config_json) {
        return -1;
    }

    // Set default options
    strcpy(ssl_options.cert_path, DEFAULT_CERT_PATH);
    strcpy(ssl_options.key_path, DEFAULT_KEY_PATH);

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
            strcpy(ssl_options.cert_path, cert_path->valuestring);
        }

        key_path = cJSON_GetObjectItemCaseSensitive(ssl_obj, "key");
        if (NULL != key_path) {
            if (-1 == check_config_option(key_path, STRING, "ssl.key")) {
                return -1;
            }
            strcpy(ssl_options.key_path, key_path->valuestring);
        }
    }

    return 0;
}

