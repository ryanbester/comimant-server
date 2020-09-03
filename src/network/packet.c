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

#include "../log.h"
#include "packet.h"
#include "../db/auth_tokens.h"

#include "packet/packet_files.h"

uint64_t
get_packet_length(const uint8_t *bytes, channel_t *channel)
{
    packet_t *packet = malloc(sizeof(packet_t));
    memset(packet, 0, sizeof(*packet));
    channel->packet = packet;

    packet_header_t *packet_header = malloc(sizeof(packet_header_t));
    memset(packet_header, 0, sizeof(*packet_header));
    packet->header = packet_header;

    if (bytes[0] != 0x43 || bytes[1] != 0x50) {
        return -1;
    }

    packet_header->protocol_version = (bytes[2] << 8) + bytes[3];

    if (bytes[4] != 0x01 && bytes[4] != 0x02) {
        // Packet type invalid
        return -1;
    }
    packet_header->packet_type = bytes[4];

    uint8_t length_bytes[5];
    memcpy(length_bytes, &bytes[5], 5);

    uint64_t length;
    decode_uvarint(length_bytes, &length);
    channel->bytes_to_read = length - 10; // Take 10 to get the remaining length

    packet_header->length = length;
    return length;
}

int
deserialize_request(const uint8_t *bytes, channel_t *channel)
{
    packet_header_t *packet_header = channel->packet->header;
    packet_header->header_type = HEADER_TYPE_REQUEST;

    uint8_t length_bytes[5];
    memcpy(length_bytes, &bytes[5], 5);

    int index = decode_uvarint(length_bytes, NULL) + 5;

    packet_header->packet_id = bytes[index++];
    packet_header->verb = bytes[index++];

    uint8_t unique_id_bytes[5];
    memcpy(unique_id_bytes, &bytes[index], 5);
    index += decode_uvarint(unique_id_bytes, &packet_header->unique_id);

    packet_header->authorization = bytes[index++];
    memcpy(packet_header->authorization_token, &bytes[index], 16);
    index += 16;

    index++; // TODO: Implement extensions

    // Authorization
    packet_header->auth_details = NULL;
    if (0x01 == packet_header->authorization) {
        auth_details_t *auth_details = malloc(sizeof(auth_details_t));
        packet_header->auth_details = auth_details;

        if (-1 == check_auth_token(packet_header->authorization_token, auth_details)) {
            // Access denied or database error
            return -1;
        }

        if (-1 == delete_auth_token(packet_header->authorization_token)) {
            return -1;
        }
    }

    if (packet_header->packet_type == PACKET_TYPE_FIXED) {
        packet_header->encoding = bytes[index++];
    } else if (packet_header->packet_type == PACKET_TYPE_STREAM) {
        uint8_t cont_id_bytes[5];
        memcpy(cont_id_bytes, &bytes[index], 5);
        index += decode_uvarint(cont_id_bytes, &packet_header->continuation_id);
    }

    channel->packet->body_index = index;

    switch (packet_header->packet_id) {
        case 0x01:
            handle_files_packet(bytes, channel);
            break;
        default:
            log_info("Unknown packet");
            break;
    }

    if (NULL != channel->packet->deserialize_request) {
        if (-1 == channel->packet->deserialize_request(bytes, channel)) {
            return -1;
        }

        return 0;
    }

    return -1;
}

int
process_data(channel_t *channel)
{
    if (NULL != channel->packet->process_data) {
        if (-1 == channel->packet->process_data(channel)) {
            return -1;
        }

        return 0;
    }

    return 0;
}


const uint8_t *
serialize_response(channel_t *channel)
{
    if (NULL != channel->packet->serialize_response) {
        return channel->packet->serialize_response(channel);
    }

    return "NULL";
}

void
free_packet(channel_t *channel)
{
    free(channel->packet->header->auth_details);
    free(channel->packet->header);
    free(channel->packet);
}

int
decode_uvarint(const uint8_t *bytes, uint64_t *buffer)
{
    int i = 0;
    uint64_t decoded_value = 0;
    int shift_amount = 0;

    do {
        decoded_value |= (uint64_t) (bytes[i] & 0x7F) << shift_amount;
        shift_amount += 7;
    } while ((bytes[i++] & 0x80) != 0);

    if (NULL != buffer) {
        *buffer = decoded_value;
    }

    return i;
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
