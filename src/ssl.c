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

#include "main.h"
#include "ssl.h"
#include "log.h"

void
init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void
cleanup_openssl() {
    EVP_cleanup();
}

SSL_CTX *
create_context() {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = SSLv23_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        log_error("Unable to craete SSL context");
        return NULL;
    }

    return ctx;
}

int
configure_context(SSL_CTX *ctx) {
    SSL_CTX_set_ecdh_auto(ctx, 1);

    if (SSL_CTX_use_certificate_file(ctx, ssl_options.cert_path, SSL_FILETYPE_PEM) <= 0) {
        log_error("Cannot load certificate PEM file: %s", ssl_options.cert_path);
        return -1;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, ssl_options.key_path, SSL_FILETYPE_PEM) <= 0) {
        log_error("Cannot load private key PEM file: %s", ssl_options.key_path);
        return -1;
    }

    return 0;
}
