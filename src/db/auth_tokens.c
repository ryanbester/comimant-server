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

#include "auth_tokens.h"

#include <string.h>
#include <mysql.h>
#include <time.h>

#include "database.h"
#include "../log.h"
#include "../config/config_info.h"

int
check_auth_token(uint8_t *auth_token, auth_details_t *details)
{
    MYSQL *conn = NULL;
    if (-1 == database_connect(&conn, NORMAL)) {
        log_error("Error connecting to database");
        return -1;
    }

    char query[70] = "SELECT * FROM server_auth_tokens WHERE auth_token = '";
    mysql_real_escape_string(conn, &query[strlen(query)], (const char *) auth_token, 16);
    strcpy(&query[strlen(query)], "'");

    if (0 != mysql_query(conn, query)) {
        log_error("MySQL Error: %s", mysql_error(conn));
        database_close(conn);
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (NULL == result) {
        log_error("MySQL Error: %s", mysql_error(conn));
        database_close(conn);
        return -1;
    }

    if (mysql_num_rows(result) <= 0) {
        log_error("No auth tokens in database");
        mysql_free_result(result);
        database_close(conn);
        return -1;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        if (0 == memcmp(row[0], auth_token, 16)) {
            mysql_free_result(result);
            database_close(conn);

            if (0 != strcmp(config_info.server_name, row[1])) {
                return -1;
            }

            // Convert time to timestamp
            struct tm expires;
            if (NULL == strptime(row[3], "%Y-%m-%d %H:%M:%S", &expires)) {
                return -1;
            }

            if (difftime(time(NULL), mktime(&expires)) > 0) {
                return -1;
            }

            memset(details, 0, sizeof(*details));
            memcpy(details->user_id, row[2], 16);
            memcpy(details->scope, row[4], strlen(row[4]));
            return 0;
        }
    }

    mysql_free_result(result);
    database_close(conn);
    return -1;
}

int
delete_auth_token(uint8_t *auth_token)
{
    MYSQL *conn = NULL;
    if (-1 == database_connect(&conn, DELETE)) {
        log_error("Error connecting to database");
        return -1;
    }

    char query[68] = "DELETE FROM server_auth_tokens WHERE auth_token = '";
    mysql_real_escape_string(conn, &query[strlen(query)], (const char *) auth_token, 16);
    strcpy(&query[strlen(query)], "'");

    if (0 != mysql_query(conn, query)) {
        log_error("MySQL Error: %s", mysql_error(conn));
        database_close(conn);
        return -1;
    }

    database_close(conn);
    return 0;
}
