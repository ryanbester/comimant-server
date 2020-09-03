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

#ifndef _PACKET_FILES_H_
#define _PACKET_FILES_H_

#include "../packet.h"

typedef struct retrieve_files_body {
    uint8_t file_server_name[32];
    uint8_t directory[4096];
} retrieve_files_body_t;

void
handle_files_packet(const uint8_t *bytes, channel_t *channel);

int
deserialize_retrieve_files_packet(const uint8_t *bytes, channel_t *channel);

const uint8_t *
serialize_retrieve_files_packet(channel_t *channel);

void
retrieve_files_free(channel_t *channel);

#endif //_PACKET_FILES_H_
