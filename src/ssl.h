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

#ifndef _SSL_H_
#define _SSL_H_

#include <openssl/ssl.h>
#include <openssl/err.h>

/**
 * Initialises OpenSSL, loading the strings and SSL algorithms.
 */
void
init_openssl();

/**
 * Cleans up OpenSSL.
 */
void
cleanup_openssl();

/**
 * Creates a new SSL context.
 * @return The SSL context structure.
 */
SSL_CTX *
create_context();

/**
 * Configures an SSL context.
 * @param ctx[in] The SSL context structure.
 */
int
configure_context(SSL_CTX *ctx);

#endif //_SSL_H_
