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

#include "database.h"
#include "../config/config_database.h"
#include "../log.h"

#include <string.h>
#include <mysql.h>

int
database_connect(MYSQL **conn, database_mode_t mode)
{
    *conn = mysql_init(NULL);
    if (NULL == *conn) {
        log_error("Failed to initialize connection to MySQL database: %s", mysql_error(*conn));
        return -1;
    }

    char username[32];
    char password[32];

    switch (mode) {
        case MODIFY:
            strcpy(username, database_options.user_modify);
            strcpy(password, database_options.password_modify);
            break;
        case DELETE:
            strcpy(username, database_options.user_delete);
            strcpy(password, database_options.password_delete);
            break;
        default:
            strcpy(username, database_options.user);
            strcpy(password, database_options.password);
    }

    if (NULL == mysql_real_connect(*conn, database_options.host, username, password,
                                   database_options.database, database_options.port, NULL, 0)) {
        log_error("Failed to connect to MySQL database: %s", mysql_error(*conn));
        mysql_close(*conn);
        return -1;
    }

    return 0;
}

void
database_close(MYSQL *conn)
{
    if (NULL != conn) {
        mysql_close(conn);
    }
}
