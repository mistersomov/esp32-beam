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

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STX_BYTE 0x24        ///< Start-of-Frame delimiter ('$')
#define MAX_PAYLOAD_SIZE 250 ///< Maximum size of the data payload in bytes

/**
 * @brief Beam Protocol Frame Structure
 *
 * Represents the full packet as it is sent over the transport layer
 * Its purpose is to extract a coherent, valid message from a continuous stream of bytes (which may contain interference
 * or interruptions)
 */
typedef struct beam_frame {
    uint8_t stx;                       ///< Start-of-Frame delimiter
    uint8_t msg_id;                    ///< Unique identifier for the message type
    uint8_t seq;                       ///< Packet sequence number for loss tracking
    uint8_t len;                       ///< Length of the payload array (0 to MAX_PAYLOAD_SIZE)
    uint8_t payload[MAX_PAYLOAD_SIZE]; ///< Raw data bytes
    uint16_t crc;                      ///< CRC-16-CCITT checksum for error detection
} beam_frame_t;

#ifdef __cplusplus
}
#endif

#endif /* BEAM_FRAME_H */