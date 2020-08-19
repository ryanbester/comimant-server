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
 * @brief Daemon.h
 */

#ifndef _DAEMON_H_
#define _DAEMON_H_

/**
 * Daemon structure
 */
typedef struct {
    int pid; /**< The PID */
} daemon_t;

/**
 * Gets the PID from the PID file.
 * @param[in] pid_file The PID file path .
 * @return The PID, or -1 on error or if the daemon isn't running.
 */
int
get_pid(const char *pid_file);

/**
 * Signal handler. This function should not be called directory.
 * @param[in] s The signal.
 */
void
sig_handler(int s);

/**
 * Starts the daemon.
 * @param[in] pid_file The PID file path.
 * @return 0 on success or -1 on error.
 */
int
daemon_start_server(const char *pid_file);

/**
 * Stops the daemon. This function should always succeed.
 * @param[in] daemon The daemon structure.
 * @param[in] pid_file The PID file path.
 * @return 0 on success or -1 on error.
 */
int
daemon_stop_server(daemon_t *daemon, const char *pid_file);

/**
 * Force stops the daemon. This function should always succeed.
 * @param[in] daemon The daemon structure.
 * @param[in] pid_file The PID file path.
 * @return 0 on success or -1 on error.
 */
int
daemon_force_stop_server(daemon_t *daemon, const char *pid_file);

#endif //_DAEMON_H_
