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

#include "packet_files.h"
#include "../../../include/log/log.h"

void
handle_files_packet(const uint8_t *bytes, channel_t *channel)
{
    switch (channel->packet->header->verb) {
        case 0x01:
            channel->packet->deserialize_request = deserialize_retrieve_files_packet;
            channel->packet->serialize_response = serialize_retrieve_files_packet;
            channel->packet->free_data = retrieve_files_free;
            break;
        default:
            break;
    }
}

int
deserialize_retrieve_files_packet(const uint8_t *bytes, channel_t *channel)
{
    uint8_t scope[16] = "test";
    if (0 != memcmp(channel->packet->header->auth_details->scope, scope, 16)) {
        log_error("Invalid scope");
        return -1;
    }

    strcpy(channel->packet->packet_name, "Files:RetrieveFiles");

    retrieve_files_body_t *body = malloc(sizeof(*body));
    channel->packet->body = body;

    int index = channel->packet->body_index;

    // TODO: Support different encodings

    // TODO: Tidy this code:
    memccpy(body->file_server_name, &bytes[index], '\0', 32);
    index += strlen((char *) body->file_server_name) + 1;
    memccpy(body->directory, &bytes[index], '\0', 4096);

    return 0;
}

const uint8_t *
serialize_retrieve_files_packet(channel_t *channel)
{
    log_info("Serialize packet: %s", channel->packet->packet_name);
    log_info("File server name: %s", ((retrieve_files_body_t *) channel->packet->body)->file_server_name);
    log_info("Directory: %s", ((retrieve_files_body_t *) channel->packet->body)->directory);

    return "Test";
}

void
retrieve_files_free(channel_t *channel)
{
    if (NULL != channel->packet->body) {
        free(channel->packet->body);
    }
}
