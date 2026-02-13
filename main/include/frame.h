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

#include "message_common.h"
#include "payload_type.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Total frame size for given payload length */
#define FRAME_SIZE(len) ((size_t)FRAME_HEADER_SIZE + (size_t)(len) + (size_t)FRAME_CRC_SIZE)
#define FRAME_MIN_SIZE FRAME_SIZE(0u) ///< Minimum frame size in bytes (header 4 + payload 0 + CRC 2).
#define FRAME_HEADER_SIZE 4u          ///< Header bytes: msg_id + flags + seq + len
#define FRAME_CRC_SIZE 2u             ///< CRC size in bytes

/**
 * @brief BEAM frame header (4 bytes on wire: msg_id, flags, seq, len).
 */
typedef struct __attribute__((packed)) beam_frame_header {
    beam_msg_id_t msg_id; ///< Unique identifier for the message type
    beam_flags_t flags;   ///< Bit mask (priority, requires ACK, etc.)
    uint8_t seq;          ///< Packet sequence number for loss tracking
    uint8_t len;          ///< Length of the payload array (0 to MAX_PAYLOAD_SIZE)
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