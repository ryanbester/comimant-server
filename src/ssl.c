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
#include "config/config_ssl.h"
#include "ssl.h"
#include "log.h"

/**
 * SSL key logging file path
 */
FILE *ssl_keylog_fp;

void
init_openssl()
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void
cleanup_openssl()
{
    EVP_cleanup();
}

void
init_keylogging(SSL_CTX *ctx)
{
    char *ssl_keylog_file = getenv("SSLKEYLOGFILE");
    if (NULL != ssl_keylog_file) {
        // At least OpenSSL version 1.1.0 is required for key logging.
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
        // Grab a file pointer to the key log file
        char ssl_keylog_file_abs[4096];
        realpath(ssl_keylog_file, ssl_keylog_file_abs);
        ssl_keylog_fp = fopen(ssl_keylog_file_abs, "a+");
        if (NULL == ssl_keylog_fp) {
            log_error("[SSL Keylog] Failed to open SSL key log file: %s\n", ssl_keylog_file_abs);
            return;
        }

        SSL_CTX_set_keylog_callback(ctx, keylog_callback);
#else
        log_error("Key logging is only available for OpenSSL version 1.1.0 or greater");
#endif
    }
}

void
keylog_callback(const SSL *ssl, const char *line)
{
    fprintf(ssl_keylog_fp, "%s\n", line);
    fflush(ssl_keylog_fp);
}

SSL_CTX *
create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = SSLv23_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        log_error("Unable to create SSL context");
        return NULL;
    }

    return ctx;
}

int
configure_context(SSL_CTX *ctx)
{
    SSL_CTX_set_ecdh_auto(ctx, 1);

    if (SSL_CTX_use_certificate_file(ctx, ssl_options.cert_path, SSL_FILETYPE_PEM) <= 0) {
        log_error("Cannot load certificate PEM file: %s", ssl_options.cert_path);
        return -1;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, ssl_options.key_path, SSL_FILETYPE_PEM) <= 0) {
        log_error("Cannot load private key PEM file: %s", ssl_options.key_path);
        return -1;
    }

    // Enable key logging
    init_keylogging(ctx);

    return 0;
}

void
SSL_cleanup_context(SSL_CTX *ctx)
{
    fclose(ssl_keylog_fp);
}
