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

#include "parser.h"
#include "esp_check.h"
#include <string.h>

#define FRAME_HEADER_SIZE 3u /**< Header bytes: msg_id + seq + len */
#define FRAME_CRC_SIZE 2u
#define CRC_INIT 0xFFFFu
#define CRC_BIT_MASK 0x8000
#define CRC_POLYNOM 0x1021

static const char *TAG = "[BEAM_parser]";

/**
 * If condition is false, log msg and return ret_val.
 * Pass the condition that must hold to continue (true = do not return).
 */
#define PARSER_RETURN_ON_FALSE(condition, msg, ret_val) ESP_RETURN_ON_FALSE(condition, ret_val, TAG, "%s", msg)

/**
 * @brief Update CRC-16-CCITT with one byte (MSB first, polynomial 0x1021).
 *
 * @param crc Current CRC (use CRC_INIT for the first byte).
 * @param data Next input byte.
 *
 * @return Updated 16-bit CRC.
 */
static uint16_t beam_compute_crc16_per_byte(uint16_t crc, uint8_t data)
{
    crc ^= (uint16_t)data << 8;
    for (uint8_t i = 0; i < 8; i++) {
        if (crc & CRC_BIT_MASK)
            crc = (crc << 1) ^ CRC_POLYNOM;
        else
            crc <<= 1;
    }

    return crc;
}

/**
 * @brief Compute frame CRC over header (msg_id, seq, len) and payload.
 *
 * @param data Buffer starting with the 3-byte header; length at least FRAME_HEADER_SIZE + payload_len.
 * @param payload_len Number of payload bytes to include in CRC.
 *
 * @return CRC-16-CCITT value (same byte order as on wire: LSB first).
 */
static uint16_t beam_compute_frame_crc(const uint8_t *data, size_t payload_len)
{
    uint16_t crc = CRC_INIT;

    crc = beam_compute_crc16_per_byte(crc, data[0]); // msg_id
    crc = beam_compute_crc16_per_byte(crc, data[1]); // seq
    crc = beam_compute_crc16_per_byte(crc, data[2]); // len
    for (size_t i = 0; i < payload_len; i++) {
        crc = beam_compute_crc16_per_byte(crc, data[FRAME_HEADER_SIZE + i]);
    }

    return crc;
}

/**
 * @brief Parse and validate a raw frame buffer into out.
 *
 * Caller must ensure data_len >= BEAM_FRAME_MIN_SIZE and non-NULL arguments.
 *
 * @return ESP_OK on success; ESP_ERR_INVALID_SIZE or ESP_ERR_INVALID_CRC on failure.
 */
static esp_err_t beam_parse_frame_into(const uint8_t *data, size_t data_len, beam_frame_t *out)
{
    PARSER_RETURN_ON_FALSE(data_len >= FRAME_HEADER_SIZE,
                           "buffer shorter than frame header (3 bytes)",
                           ESP_ERR_INVALID_SIZE);

    uint8_t len = data[2];
    PARSER_RETURN_ON_FALSE(len <= MAX_PAYLOAD_SIZE, "payload length exceeds MAX_PAYLOAD_SIZE", ESP_ERR_INVALID_SIZE);

    size_t frame_size = (size_t)FRAME_HEADER_SIZE + (size_t)len + FRAME_CRC_SIZE;
    PARSER_RETURN_ON_FALSE(data_len >= frame_size, "buffer shorter than header + payload + CRC", ESP_ERR_INVALID_SIZE);

    uint16_t expected_crc = beam_compute_frame_crc(data, len);
    uint16_t received_crc =
        (uint16_t)data[FRAME_HEADER_SIZE + len] | ((uint16_t)data[FRAME_HEADER_SIZE + len + 1] << 8);
    PARSER_RETURN_ON_FALSE(expected_crc == received_crc, "frame CRC mismatch", ESP_ERR_INVALID_CRC);

    out->header.msg_id = data[0];
    out->header.seq = data[1];
    out->header.len = len;
    memcpy(out->payload, data + FRAME_HEADER_SIZE, len);
    out->crc = received_crc;

    return ESP_OK;
}

esp_err_t beam_validate_frame(const uint8_t *data, size_t data_len, beam_frame_t *out_frame)
{
    PARSER_RETURN_ON_FALSE(data != NULL, "data pointer is NULL", ESP_ERR_INVALID_ARG);
    PARSER_RETURN_ON_FALSE(out_frame != NULL, "out_frame pointer is NULL", ESP_ERR_INVALID_ARG);
    PARSER_RETURN_ON_FALSE(data_len >= BEAM_FRAME_MIN_SIZE,
                           "data_len less than BEAM_FRAME_MIN_SIZE (5)",
                           ESP_ERR_INVALID_SIZE);

    return beam_parse_frame_into(data, data_len, out_frame);
}
