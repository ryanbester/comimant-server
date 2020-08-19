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

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <poll.h>

#include "worker.h"
#include "config/config_listen.h"
#include "ssl.h"
#include "plugins/plugins.h"

#include "config/config.h"

#include "../include/log/log.h"

#define MAX_FDS 1024

/**
 * Store the file descriptors of the TCP and UNIX listeners.
 */
int tcp_fd = -1, unix_fd = -1;

/**
 * Store the epoll file descriptor.
 */
int epoll_fd = -1;

/**
 * Store the SSL context.
 */
SSL_CTX *ctx;

void
channel_update(channel_t *ch)
{
    struct epoll_event ev = {0};
    ev.events = ch->events;
    ev.data.ptr = ch;
    if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_MOD, ch->fd, &ev)) {
        log_error("Epoll_ctl modify failed");
    }
}

void
channel_close(channel_t *ch)
{
    close(ch->fd);
    if (NULL != ch->ssl) {
        SSL_shutdown(ch->ssl);
        SSL_free(ch->ssl);
    }
    free(ch);
    log_info("Freed ram");
}

void *
get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

int
set_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }

    flags |= O_NONBLOCK;
    if (-1 == fcntl(fd, F_SETFL, flags)) {
        return -1;
    }

    return 0;
}

int
get_unix_fd()
{
    if (1 == listen_options.lo_unix_enabled) {
        log_info("Starting unix listener...");

        struct sockaddr_un local, remote;

        if (-1 == (unix_fd = socket(AF_UNIX, SOCK_STREAM, 0))) {
            log_error("[Worker] Cannot create socket");
            return -1;
        }

        local.sun_family = AF_UNIX;
        strcpy(local.sun_path, listen_options.lo_unix.sock_file);

        // Delete old socket file
        if (-1 == remove(local.sun_path)) {
            // Ignore if error is non existent file
            if (errno != ENOENT) {
                log_error("[Worker] Cannot delete existing socket file: %s", local.sun_path);
                return -1;
            }
        }

        size_t len = strlen(local.sun_path) + sizeof(local.sun_family);
        if (-1 == bind(unix_fd, (struct sockaddr *) &local, len)) {
            log_error("[Worker] Cannot bind socket");
            return -1;
        }

        // Make non blocking
        if (-1 == set_non_blocking(unix_fd)) {
            log_error("[Worker] Error putting UNIX socket in non blocking mode");
            return -1;
        }

        if (-1 == listen(unix_fd, listen_options.lo_unix.backlog)) {
            log_error("[Worker] Cannot listen on socket");
            return -1;
        }
    }

    return unix_fd;
}

int
get_tcp_fd()
{
    if (1 == listen_options.lo_tcp_enabled) {
        // Start TCP listener
        log_info("Starting TCP listener...");

        struct addrinfo hints, *servinfo, *p;
        int yes = 1;

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        int ret;
        char port[6];
        sprintf(port, "%d", listen_options.lo_tcp.port);
        if ((ret = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
            log_error("[Worker] Error getting address information for TCP listener.");
            return -1;
        }

        for (p = servinfo; p != NULL; p = p->ai_next) {
            if (-1 == (tcp_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))) {
                log_error("[Worker] Cannot create TCP socket");
                continue;
            }

            if (-1 == setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
                log_error("[Worker] Cannot setsockopt");
                continue;
            }

            if (-1 == bind(tcp_fd, p->ai_addr, p->ai_addrlen)) {
                close(tcp_fd);
                log_error("[Worker] Cannot bind TCP socket");
                continue;
            }

            break;
        }

        freeaddrinfo(servinfo);

        if (p == NULL) {
            log_error("[Worker] Failed to bind TCP server");
            return -1;
        }

        // Make non blocking
        if (-1 == set_non_blocking(tcp_fd)) {
            log_error("[Worker] Error putting TCP socket in non blocking mode");
            return -1;
        }

        if (listen(tcp_fd, listen_options.lo_tcp.backlog) == -1) {
            log_error("[Worker] Failed to listen on TCP socket");
            return -1;
        }
    }

    return tcp_fd;
}

void
handle_unix_accept()
{
    char remote_ip[INET6_ADDRSTRLEN];
    struct sockaddr_storage remote_addr;

    socklen_t addr_len = sizeof remote_addr;
    int c_fd = accept(unix_fd, (struct sockaddr *) &remote_addr, &addr_len);

    if (c_fd < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            log_warn("Accept return EAGAIN or EWOULDBLOCK");
        } else {
            log_error("[Worker] Error accepting");
        }
    } else {
        set_non_blocking(c_fd);
        if (c_fd >= MAX_FDS) {
            log_error("Maximum file descriptor limit reached");
            return;
        }

        log_info("[Worker] Got connection on UNIX socket from %s",
                 inet_ntop(remote_addr.ss_family, get_in_addr((struct sockaddr *) &remote_addr), remote_ip,
                           INET6_ADDRSTRLEN));

        channel_t *new_ch = malloc(sizeof(channel_t));
        new_ch->fd = c_fd;

        struct epoll_event ev = {0};
        ev.data.ptr = new_ch;
        ev.events = EPOLLIN | EPOLLOUT;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, c_fd, &ev) < 0) {
            log_error("Error adding new socket to epoll");
        }
    }
}

void
handle_tcp_accept()
{
    char remote_ip[INET6_ADDRSTRLEN];
    struct sockaddr_storage remote_addr;

    socklen_t addr_len = sizeof remote_addr;
    int c_fd = accept(tcp_fd, (struct sockaddr *) &remote_addr, &addr_len);

    if (c_fd < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            log_warn("Accept return EAGAIN or EWOULDBLOCK");
        } else {
            log_error("[Worker] Error accepting");
        }
    } else {
        set_non_blocking(c_fd);
        if (c_fd >= MAX_FDS) {
            log_error("Maximum file descriptor limit reached");
            return;
        }

        log_info("[Worker] Got connection on TCP socket from %s",
                 inet_ntop(remote_addr.ss_family, get_in_addr((struct sockaddr *) &remote_addr), remote_ip,
                           INET6_ADDRSTRLEN));

        channel_t *new_ch = malloc(sizeof(channel_t));
        new_ch->fd = c_fd;

        struct epoll_event ev = {0};
        ev.data.ptr = new_ch;
        ev.events = EPOLLIN | EPOLLOUT;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, c_fd, &ev) < 0) {
            log_error("Error adding new socket to epoll");
        }
    }
}

void
handle_handshake(channel_t *ch)
{
    if (0 == ch->net_connected) {
        // Wait until file descriptor is ready for writing data to
        struct pollfd p_fd;
        p_fd.fd = ch->fd;
        p_fd.events = POLLOUT | POLLERR;
        int r = poll(&p_fd, 1, 0);
        if (1 == r && p_fd.revents == POLLOUT) {
            log_info("TCP/UNIX connected fd: %d", ch->fd);
            ch->net_connected = 1;
            ch->events = EPOLLIN | EPOLLOUT | EPOLLERR;
            channel_update(ch);
        } else {
            log_warn("poll fd %d return %d revents %d", ch->fd, r, p_fd.revents);
            channel_close(ch);
            return;
        }
    }

    if (NULL == ch->ssl) {
        // Initialise SSL object
        ch->ssl = SSL_new(ctx);
        if (NULL == ch->ssl) {
            log_error("SSL_new failed");
        }
        int r = SSL_set_fd(ch->ssl, ch->fd);
        if (r == 0) {
            log_error("SSL_set_fd failed");
        }
        log_info("SSL_set_accept_state for fd %fd", ch->fd);
        SSL_set_accept_state(ch->ssl);
    }

    // Establish SSL handshake
    int r = SSL_do_handshake(ch->ssl);
    if (1 == r) {
        ch->ssl_connected = true;
        log_info("SSL connected fd %d", ch->fd);
        return;
    }

    int err = SSL_get_error(ch->ssl, r);
    int old_ev = ch->events;

    switch (err) {
        case SSL_ERROR_WANT_WRITE:
            ch->events |= EPOLLOUT;
            ch->events &= ~EPOLLIN;
            log_info("return want write set events %d", ch->events);
            channel_update(ch);
            break;
        case SSL_ERROR_WANT_READ:
            ch->events |= EPOLLIN;
            ch->events &= ~EPOLLOUT;
            log_info("return want read set events %d", ch->events);
            channel_update(ch);
            break;
        default:
            log_error("SSL_do_handshake return %d error %d errno %d msg %s", r, err, errno, strerror(errno));
            channel_close(ch);
    }
}

void
handle_data_read(channel_t *ch)
{
    char buf[4096] = {0};
    int rd = SSL_read(ch->ssl, buf, sizeof(buf));
    int ssl_err = SSL_get_error(ch->ssl, rd);
    if (rd > 0) {
        if (0 == strncmp(buf, "close", 5)) {
            const char *msg = "Closing connection";
            int wd = SSL_write(ch->ssl, msg, strlen(msg));
            log_info("SSL_Write %d bytes", wd);
            channel_close(ch);
            return;
        }
        int wd = SSL_write(ch->ssl, buf, strlen(buf));
        log_info("SSL_Write %d bytes", wd);
    }
    if (rd < 0 && ssl_err != SSL_ERROR_WANT_READ) {
        log_error("SSL_read return %d error %d errno %d msg %s", rd, ssl_err, errno, strerror(errno));
        channel_close(ch);
        return;
    }
    if (0 == rd) {
        if (ssl_err == SSL_ERROR_ZERO_RETURN) {
            log_info("SSL has been shutdown");
        } else {
            log_info("Connected has been aborted.");
        }
        channel_close(ch);
    }
}

void
socket_read(channel_t *ch)
{
    if (ch->fd == unix_fd) {
        // Handle accept on UNIX socket
        return handle_unix_accept();
    }
    if (ch->fd == tcp_fd) {
        // Handle accept on TCP socket
        return handle_tcp_accept();
    }
    if (1 == ch->ssl_connected) {
        // Read data if SSL handshake has been established
        return handle_data_read(ch);
    }

    // Otherwise establish the SSL connection
    handle_handshake(ch);
}

void
socket_write(channel_t *ch)
{
    if (0 == ch->ssl_connected) {
        // Establish an SSL connection
        return handle_handshake(ch);
    }

    // Write to socket
    log_trace("Write to socket: %d", ch->fd);
    ch->events &= ~EPOLLOUT;
    channel_update(ch);
}

int
perform_loop_cycle()
{
    struct epoll_event events[MAX_FDS];

    int n_ready = epoll_wait(epoll_fd, events, MAX_FDS, -1);
    for (int i = 0; i < n_ready; i++) {
        channel_t *ch = (channel_t *) events[i].data.ptr;

        if (events[i].events & (EPOLLIN | EPOLLERR)) {
            socket_read(ch);
        } else if (events[i].events & EPOLLOUT) {
            socket_write(ch);
        } else {
            log_warn("Unknown event");
        }
    }
}

int
worker_listen()
{
    fd_set master, read_fds;
    int socket_count = 0;
    int fd_max;

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    get_unix_fd();
    if (unix_fd > -1) {
        // Add unix listener to master set
        FD_SET(unix_fd, &master);
        fd_max = unix_fd;

        socket_count++;
    }

    get_tcp_fd();
    if (tcp_fd > -1) {
        // Add TCP listener to master set
        FD_SET(tcp_fd, &master);

        // Increase fd_max if it is larger than UNIX listener
        if (tcp_fd > unix_fd) {
            fd_max = tcp_fd;
        }

        socket_count++;
    }

    if (0 == socket_count) {
        log_info("[Worker] Nothing to listen on. Terminating...");
        return -1;
    }

    // Setup SSL
    ctx = create_context();
    configure_context(ctx);

    // epoll
    log_info("Running...");

    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        log_error("[Worker] Error creating epoll");
        return -1;
    }

    channel_t *unix_ch = NULL;
    channel_t *tcp_ch = NULL;

    if (unix_fd > -1) {
        unix_ch = malloc(sizeof(channel_t));
        unix_ch->fd = unix_fd;

        struct epoll_event unix_ev;
        unix_ev.data.ptr = unix_ch;
        unix_ev.events = EPOLLIN;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, unix_fd, &unix_ev) < 0) {
            log_error("[Worker] Error adding UNIX socket to epoll");
            return -1;
        }
    }

    if (tcp_fd > -1) {
        tcp_ch = malloc(sizeof(channel_t));
        tcp_ch->fd = tcp_fd;

        struct epoll_event tcp_ev;
        tcp_ev.data.ptr = tcp_ch;
        tcp_ev.events = EPOLLIN;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tcp_fd, &tcp_ev) < 0) {
            log_error("[Worker] Error adding TCP socket to epoll");
            return -1;
        }
    }

    loop_running = 1;
    while (loop_running == 1) {
        perform_loop_cycle();
    }

    SSL_cleanup_context(ctx);
    free(unix_ch);
    free(tcp_ch);
    deactivate_plugins();
    free_plugins();
    return 0;
}

