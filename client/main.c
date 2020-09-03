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

#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "main.h"

#include "../include/log/log.h"

#define SOCK_FILE "comimant.sock"

void
show_opt_err();

void
show_help();

const uint8_t *
load_data(int *length)
{
    int index;

    // Set data to send here:
    static uint8_t data[256];
    data[0] = 0x43; // Identifier
    data[1] = 0x50; // Identifier

    data[2] = 0x00; // |
    data[3] = 0x01; // Version
    index = 4;

    data[index] = 0x01; // Packet Type (Fixed)
    index++;

    static uint8_t header_data[256];
    int header_index;

    header_data[0] = 0x01; // Packet ID
    header_data[1] = 0x01; // Verb

    // TODO: Use memcpy instead of strcpy
    uint8_t unique_id[5];
    int unique_id_bytes = encode_uvarint(unique_id, 500);
    strcpy(&header_data[2], unique_id); // Unique ID
    header_index = 2 + unique_id_bytes;

    header_data[header_index] = 0x01; // Enable auth
    strcpy(&header_data[header_index + 1], "abcdefgh12345678"); // Auth token
    header_index += 17;

    header_data[header_index] = 0x00; // Extensions
    header_index++;

    header_data[header_index] = 0x01; // Encoding
    header_index++;

    static uint8_t body_data[256];
    int body_index;
    // Body:
    strcpy(&body_data[0], "shared\0"); // File server name
    strcpy(&body_data[7], "/\0"); // Directory
    body_index = 9;


    uint8_t header_length[5];
    int header_length_bytes = encode_uvarint(header_length, header_index + body_index);
    strcpy(&data[index], header_length); // Header length
    index += header_length_bytes;

    memcpy(&data[index], header_data, header_index);
    index += header_index;

    memcpy(&data[index], body_data, body_index);
    index += body_index;

    printf("Packet length: %d\n", index);

    *length = index;
    return data;
}

int
main(int argc, char **argv)
{
    int length;
    const uint8_t *data = load_data(&length);

    printf("Welcome to Comimant Client\n");

    // Init SSL
    SSL_CTX *ctx;

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ctx = SSL_CTX_new(TLS_client_method());
    if (NULL == ctx) {
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    // Connect to socket
    int fd, len;
    struct sockaddr_un addr;
    uint8_t buf[256];

    if ((fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        goto fail;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCK_FILE);

    if (-1 == connect(fd, (struct sockaddr *) &addr, sizeof(addr))) {
        perror("connect");
        goto fail;
    }

    // Create SSL object
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, fd);
    if (SSL_connect(ssl) != 1) {
        ERR_print_errors_fp(stderr);
    } else {
        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));

        // Print certificates
        X509 *cert = SSL_get_peer_certificate(ssl);
        if (NULL != cert) {
            printf("Server certificates: \n");
            printf("\tSubject: %s\n", X509_NAME_oneline(X509_get_subject_name(cert), 0, 0));
            printf("\tIssuer: %s\n", X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0));
            X509_free(cert);
        }

        // Send and receive data
        SSL_write(ssl, data, length);

        len = SSL_read(ssl, buf, sizeof(buf));
        if (len > 0) {
            printf("Received: %s\n", buf);
        }
    }

    fail:
    if (fd >= 0) {
        printf("Closing\n");
        SSL_free(ssl);
        close(fd);
        SSL_CTX_free(ctx);
    }

    return 0;
}

void
show_opt_err()
{
    printf("Try 'comimant-client --help' for more information.\n");
}

void
show_help()
{
    printf("Usage: comimant-client [OPTION]...\n");
}

uint64_t
decode_uvarint(const uint8_t *bytes)
{
    int i = 0;
    uint64_t decoded_value = 0;
    int shift_amount = 0;

    do {
        decoded_value |= (uint64_t) (bytes[i] & 0x7F) << shift_amount;
        shift_amount += 7;
    } while ((bytes[i++] & 0x80) != 0);

    return decoded_value;
}

int
encode_uvarint(uint8_t *buffer, uint64_t value)
{
    int encoded = 0;
    do {
        uint8_t next_byte = value & 0x7F;
        value >>= 7;

        if (value) {
            next_byte |= 0x80;
        }

        buffer[encoded++] = next_byte;
    } while (value);

    return encoded;
}

