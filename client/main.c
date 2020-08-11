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

#include "main.h"

#include "../include/log/log.h"

#define SOCK_FILE "comimant.sock"

void show_opt_err();

void show_help();

int main(int argc, char **argv) {
    printf("Welcome to Comimant Client\n");

    int fd, ret, len;
    struct sockaddr_un addr;
    char buf[256];

    if ((fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        goto fail;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCK_FILE);

    if (-1 == connect(fd, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("connect");
        goto fail;
    }

    strcpy(buf, "test");
    if (-1 == send(fd, buf, strlen(buf) + 1, 0)) {
        perror("send");
        goto fail;
    }

    printf("Sent test message\n");

    if ((len == recv(fd, buf, sizeof buf, 0)) < 0) {
        perror("recv");
        goto fail;
    }

    printf("receive %d %s\n", len, buf);

    fail:
    if (fd >= 0) {
        close(fd);
    }

    return 0;
}

void show_opt_err() {
    printf("Try 'comimant-client --help' for more information.\n");
}

void show_help() {
    printf("Usage: comimant-client [OPTION]...\n");
}
