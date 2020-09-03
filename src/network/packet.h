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

#ifndef _PACKET_H_
#define _PACKET_H_

#include "../worker.h"

#define HEADER_TYPE_REQUEST 1
#define HEADER_TYPE_RESPONSE 2

#define PACKET_TYPE_FIXED 0x01
#define PACKET_TYPE_STREAM 0x02

#define PACKET_ENCODING_UTF7 0x01
#define PACKET_ENCODING_UTF8 0x02
#define PACKET_ENCODING_UTF16 0x03

typedef struct packet packet_t;

typedef int(*deserialize_callback)(const uint8_t *bytes, channel_t *);

typedef int(*process_data_callback)(channel_t *);

typedef const uint8_t *(*serialize_callback)(channel_t *);

typedef void(*free_callback)(channel_t *);

typedef struct packet_extensions {
    // TODO: Implement
} packet_extensions_t;

typedef struct auth_details {
    uint8_t user_id[16];
    uint8_t scope[16];
} auth_details_t;

typedef struct packet_header {
    int header_type;
    unsigned short protocol_version;
    uint8_t packet_type;
    uint64_t length;
    uint8_t packet_id;
    uint8_t verb;
    uint64_t unique_id;
    uint8_t authorization;
    uint8_t authorization_token[16];
    struct packet_extensions *extensions;
    uint8_t encoding;
    uint64_t continuation_id;

    struct auth_details *auth_details;
} packet_header_t;

struct packet {
    char packet_name[32]; /**< Nice packet name for debugging */

    struct packet_header *header;
    void *body;

    int body_index; /**< Stores the index the header parser has got up to, so the body can be read. */

    deserialize_callback deserialize_request;
    process_data_callback process_data;
    serialize_callback serialize_response;
    free_callback free_data;
};

/**
 * Gets the packet length from the header length field. This does not return the actual length of the data.
 * @param bytes The first bytes of the packet up to and including the length field.
 * @return The length of the packet in bytes, or -1 if the packet is invalid.
 */
uint64_t
get_packet_length(const uint8_t *bytes, channel_t *);

/**
 * Deserializes the packet header and calls the packet specific deserialize function.
 * @param bytes
 */
int
deserialize_request(const uint8_t *bytes, channel_t *channel);

int
process_data(channel_t *channel);

/**
 * Serializes the packet header and calls the packet specific serialize function.
 * @return
 */
const uint8_t *
serialize_response(channel_t *channel);

void
free_packet(channel_t *channel);

/**
 * Decodes an unsigned VarInt.
 * @param bytes The bytes to decode.
 * @param buffer The buffer to store the decoded value.
 * @return The number of bytes of the VarInt.
 */
int
decode_uvarint(const uint8_t *bytes, uint64_t *buffer);

/**
 * Encodes an unsigned VarInt.
 * @param buffer The buffer to store the encoded bytes in.
 * @param value The value to encode.
 * @return The number of bytes of the VarInt.
 */
int
encode_uvarint(uint8_t *buffer, uint64_t value);

#endif //_PACKET_H_
