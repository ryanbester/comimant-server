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

/**
 * @file
 * @brief Worker.h
 */

#ifndef _WORKER_H_
#define _WORKER_H_

#include <openssl/ssl.h>
#include "config/config.h"


/**
 * Keeps track of whether the main listening loop is running or not.
 */
int loop_running;

/**
 * Contains data about a channel.
 */
typedef struct {
    int fd; /**< The file descriptor of the channel. */
    SSL *ssl; /**< The SSL context structure */
    int net_connected; /**< Whether the network connection has been established */
    int ssl_connected; /**< Whether the SSL handshake has been established */
    int events; /**< Stores the epoll events for convenience */
} channel_t;

/**
 * Update the channel events to the epoll event stack.
 * @param ch The channel structure.
 */
void
channel_update(channel_t *ch);

/**
 * Free a channel from memory, closing the file descriptor and discarding the SSL context.
 * @param ch The channel structure.
 */
void
channel_close(channel_t *ch);

/**
 * Gets the address form a sockaddr structure.
 * @param sa[in] The sockaddr structure.
 * @return An in6_addr or in_addr structure containing the address.
 */
void *
get_in_addr(struct sockaddr *sa);

/**
 * Puts the socket into non blocking mode.
 * @param[in] fd The socket file descriptor.
 * @return 0 on success or -1 on error.
 */
int
set_non_blocking(int fd);

/**
 * Gets the file descriptor for the UNIX socket listener.
 * @return The file descriptor or -1 if UNIX sockets are not enabled.
 */
int
get_unix_fd();

/**
 * Gets the file descriptor for the TCP listener.
 * @return The file descriptor or -1 if TCP is not enabled.
 */
int
get_tcp_fd();

/**
 * Accept a UNIX socket.
 */
void
handle_unix_accept();

/**
 * Accept a TCP socket.
 */
void
handle_tcp_accept();

/**
 * Perform an SSL handshake on a socket.
 * @param ch The channel structure.
 */
void
handle_handshake(channel_t *ch);

/**
 * Read data from a socket.
 * @param ch The channel structure.
 */
void
handle_data_read(channel_t *ch);

/**
 * Handle a read event on a file descriptor.
 * @param ch The channel structure.
 */
void
socket_read(channel_t *ch);

/**
 * Handle a write event on a file descriptor.
 * @param ch The channel structure.
 */
void
socket_write(channel_t *ch);

/**
 * Performs a single cycle of the main listening loop.
 * @return 0 on success, -1 on an error (perform next cycle), or -2 on critical error (stop the application).
 */
int
perform_loop_cycle();

/**
 * Listens on the modes specified in listen_options.
 * @return 0 on success or -1 on error.
 */
int
worker_listen();

#endif //_WORKER_H_
