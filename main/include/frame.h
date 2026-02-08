/*
 Copyright 2026 Ivan Somov

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      https://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#ifndef BEAM_FRAME_H
#define BEAM_FRAME_H

#include "payload_type.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief BEAM frame header (3 bytes on wire: msg_id, seq, len).
 */
typedef struct __attribute__((packed)) beam_frame_header {
    uint8_t msg_id; ///< Unique identifier for the message type
    uint8_t seq;    ///< Packet sequence number for loss tracking
    uint8_t len;    ///< Length of the payload array (0 to MAX_PAYLOAD_SIZE)
} beam_frame_header_t;

/**
 * @brief BEAM Protocol Frame Structure
 *
 * Represents the full packet as it is sent over the transport layer.
 * Packed so that layout and size match the wire format (no padding).
 */
typedef struct __attribute__((packed)) beam_frame {
    beam_frame_header_t header; ///< Frame header (msg_id, seq, len)
    beam_payload_t payload;     ///< Payload union; interpret by header.msg_id
    uint16_t crc;               ///< CRC-16-CCITT checksum for error detection
} beam_frame_t;

#ifdef __cplusplus
}
#endif

#endif /* BEAM_FRAME_H */