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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "daemon.h"
#include "main.h"
#include "worker.h"
#include "log.h"

int
get_pid(const char *pid_file) {
    FILE *fp = fopen(pid_file, "r");

    if (NULL == fp) {
        log_trace("[Daemon] Cannot open PID file. Assuming server is stopped.");
        return -1;
    }

    // Get file size
    fseek(fp, 0L, SEEK_END);
    size_t file_size = ftell(fp);
    rewind(fp);

    char *pid = calloc(1, file_size + 1);
    if (!pid) {
        fclose(fp);
        log_info("[Daemon] Memory allocation failed\n");
        return -1;
    }

    if (1 != fread(pid, file_size, 1, fp)) {
        fclose(fp);
        free(pid);
        log_info("[Daemon] Failed to read PID file %s\n", pid_file);
        return -1;
    }

    long pid_l = strtol(pid, NULL, 10);
    free(pid);
    return pid_l;
}

void
sig_handler(int s) {
    switch (s) {
        case SIGCHLD: {
            int saved_errno = errno;
            while (waitpid(-1, NULL, WNOHANG) > 0);
            errno = saved_errno;
            break;
        }
        case SIGINT:
        case SIGTERM: {
            log_info("Server terminating...");
            loop_running = 0;
            break;
        }
    }
}

int
daemon_start_server(const char *pid_file) {
    daemon_t daemon;
    memset(&daemon, 0, sizeof(daemon));

    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    if (-1 == sigaction(SIGCHLD, &sa, 0)) {
        log_error("Failed to register signal handler for SIGCHLD");
        return -1;
    }

    if (-1 == sigaction(SIGINT, &sa, 0)) {
        log_error("Failed to register signal handler for SIGINT");
        return -1;
    }

    if (-1 == sigaction(SIGTERM, &sa, 0)) {
        log_error("Failed to register signal handler for SIGTERM");
        return -1;
    }

    pid_t pid, sid;

    // Create a child and kill the parent
    pid = fork();
    if (pid < 0) {
        log_error("Error creating child process");
        return -1;
    }

    if (pid > 0) {
        // Kill the parent
        log_info("Child process created. Killing parent...");
        exit(0);
    }

    daemon.pid = getpid();
    log_info("Server now running as a daemon on PID: %d", daemon.pid);

    // Save PID to file
    FILE *fp = fopen(pid_file, "w");
    if (NULL == fp) {
        log_error("Failed to write to PID file");
        return -1;
    }

    fprintf(fp, "%d", daemon.pid);
    fclose(fp);

    // Set file creation mask
    umask(0);

    sid = setsid();
    if (sid < 0) {
        log_error("Error creating session");
        return -1;
    }

    if (chdir("/") < 0) {
        log_error("Error changing directory");
        return -1;
    }

    // Close unused file descriptors for security
    if (close(STDIN_FILENO) < 0) {
        log_error("Error closing stdin file descriptor");
        return -1;
    }

    if (close(STDOUT_FILENO) < 0) {
        log_error("Error closing stdout file descriptor");
        return -1;
    }

    if (close(STDERR_FILENO) < 0) {
        log_error("Error closing stderr file descriptor");
        return -1;
    }

    // Start listener
    worker_listen();

    return 0;
}

int
daemon_stop_server(daemon_t *daemon, const char *pid_file) {
    // Don't need to handle errors here.
    kill(daemon->pid, SIGTERM);
    remove(pid_file);

    return 0;
}

int
daemon_force_stop_server(daemon_t *daemon, const char *pid_file) {
    kill(daemon->pid, SIGINT);
    remove(pid_file);

    return 0;
}
