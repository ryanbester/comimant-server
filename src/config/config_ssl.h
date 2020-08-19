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

#ifndef _CONFIG_SSL_H_
#define _CONFIG_SSL_H_

#include <linux/limits.h>
#include "config_util.h"

/**
 * SSL options
 */
typedef struct {
    char cert_path[PATH_MAX]; /**< The SSL certificate path */
    char key_path[PATH_MAX]; /**< The SSL private key path */
} ssl_options_t;

ssl_options_t ssl_options;

/**
 * Default SSL certificate path
 */
#define DEFAULT_CERT_PATH "cert.pem"

/**
 * Default SSL private key path
 */
#define DEFAULT_KEY_PATH "key.pem"

/**
 * Loads the SSL options from the configuration file.
 * @param[in] config The configuration object.
 * @return 0 on success or -1 on error.
 */
int
load_ssl_options(config_file_t *config);

#endif //_CONFIG_SSL_H_
