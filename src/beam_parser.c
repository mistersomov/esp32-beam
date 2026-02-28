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

#include "beam_message_common.h"
#include "beam_parser.h"
#include "beam_payload_type.h"
#include "esp_check.h"
#include "esp_crc.h"
#include <limits.h>

#define CRC_INIT UINT16_MAX /**< Initial CRC-16-CCITT value (0xFFFF) */

static const char *TAG = "[BEAM_parser]";

/**
 * If condition is false, log msg and return ret_val.
 * Pass the condition that must hold to continue (true = do not return).
 */
#define PARSER_RETURN_ON_FALSE(condition, msg, ret_val) ESP_RETURN_ON_FALSE(condition, ret_val, TAG, "%s", msg)

/**
 * @brief Fill payload union from raw bytes according to msg_category.
 *
 * For known message types (MSG_CAT_TELEMETRY, MSG_CAT_BATTERY), copies data into
 * the typed union member if payload length is sufficient. Otherwise, or for unknown
 * types, copies into .raw array.
 *
 * @param msg_category Message type identifier.
 * @param payload_src Source buffer with payload bytes.
 * @param len Number of payload bytes to copy.
 * @param payload Destination payload union to fill.
 */
static void fill_payload(uint8_t msg_category, const uint8_t *payload_src, uint8_t len, beam_payload_t *payload)
{
    switch (msg_category) {
    case MSG_CAT_TELEMETRY:
        if (len >= sizeof(beam_payload_telemetry_t)) {
            memcpy(&payload->telemetry, payload_src, sizeof(beam_payload_telemetry_t));
        }
        else {
            memcpy(payload->raw, payload_src, len);
        }
        break;
    case MSG_CAT_BATTERY:
        if (len >= sizeof(beam_payload_battery_t)) {
            memcpy(&payload->battery, payload_src, sizeof(beam_payload_battery_t));
        }
        else {
            memcpy(payload->raw, payload_src, len);
        }
        break;
    default:
        memcpy(payload->raw, payload_src, len);
        break;
    }
}

/**
 * @brief Parse and validate a raw frame buffer into beam_frame_t structure.
 *
 * Validates frame length, payload size, and CRC. Fills header and payload fields.
 * Caller must ensure data_len >= FRAME_MIN_SIZE and non-NULL arguments.
 *
 * @param data Raw frame buffer starting with header.
 * @param data_len Length of data buffer.
 * @param out Output frame structure to fill.
 *
 * @return ESP_OK on success.
 *         ESP_ERR_INVALID_SIZE if buffer too short or payload length invalid.
 *         ESP_ERR_INVALID_CRC if CRC mismatch.
 */
static esp_err_t parse_into_frame(const uint8_t *data, size_t data_len, beam_frame_t *out)
{
    PARSER_RETURN_ON_FALSE(data_len >= FRAME_HEADER_SIZE,
                           "buffer shorter than frame header (4 bytes)",
                           ESP_ERR_INVALID_SIZE);

    uint8_t len = data[3];
    PARSER_RETURN_ON_FALSE(len <= MAX_PAYLOAD_SIZE, "payload length exceeds MAX_PAYLOAD_SIZE", ESP_ERR_INVALID_SIZE);
    PARSER_RETURN_ON_FALSE(data_len >= FRAME_SIZE(len),
                           "buffer shorter than header + payload + CRC",
                           ESP_ERR_INVALID_SIZE);

    uint16_t expected_crc = esp_crc16_be(CRC_INIT, data, FRAME_HEADER_SIZE + len);
    uint16_t received_crc =
        (uint16_t)data[FRAME_HEADER_SIZE + len] | ((uint16_t)data[FRAME_HEADER_SIZE + len + 1] << 8);
    PARSER_RETURN_ON_FALSE(expected_crc == received_crc, "frame CRC mismatch", ESP_ERR_INVALID_CRC);

    out->header.msg_category = data[0];
    out->header.flags = data[1];
    out->header.seq = data[2];
    out->header.len = len;

    fill_payload(out->header.msg_category, data + FRAME_HEADER_SIZE, len, &out->payload);

    out->crc = received_crc;

    return ESP_OK;
}

/**
 * @brief Serialize beam_frame_t into raw buffer (header + payload + CRC).
 *
 * Writes frame header, payload bytes, computes CRC over header+payload, and writes CRC.
 * CRC is written LSB first (little-endian) to match wire format.
 *
 * @param frame Frame structure to serialize. Must have valid header.len <= MAX_PAYLOAD_SIZE.
 * @param out_buffer Buffer to write serialized frame.
 * @param buffer_size Size of out_buffer in bytes.
 * @param[out] out_size Optional pointer to receive number of bytes written. Can be NULL.
 *
 * @return ESP_OK on success.
 *         ESP_ERR_INVALID_SIZE if buffer_size too small.
 */
static esp_err_t serialize_frame(const beam_frame_t *frame, uint8_t *out_buffer, size_t buffer_size, size_t *out_size)
{
    size_t required_size = FRAME_SIZE(frame->header.len);
    PARSER_RETURN_ON_FALSE(buffer_size >= required_size, "buffer_size too small for frame", ESP_ERR_INVALID_SIZE);

    out_buffer[0] = frame->header.msg_category;
    out_buffer[1] = frame->header.flags;
    out_buffer[2] = frame->header.seq;
    out_buffer[3] = frame->header.len;

    memcpy(out_buffer + FRAME_HEADER_SIZE, frame->payload.raw, frame->header.len);

    uint16_t crc = esp_crc16_be(CRC_INIT, out_buffer, FRAME_HEADER_SIZE + frame->header.len);
    size_t crc_offset = FRAME_HEADER_SIZE + frame->header.len;
    out_buffer[crc_offset] = (uint8_t)(crc & 0xFF);            // LSB
    out_buffer[crc_offset + 1] = (uint8_t)((crc >> 8) & 0xFF); // MSB

    if (out_size != NULL) {
        *out_size = required_size;
    }

    return ESP_OK;
}

esp_err_t beam_parse_into_frame(const uint8_t *data, size_t data_len, beam_frame_t *out_frame)
{
    PARSER_RETURN_ON_FALSE(data != NULL, "data pointer is NULL", ESP_ERR_INVALID_ARG);
    PARSER_RETURN_ON_FALSE(out_frame != NULL, "out_frame pointer is NULL", ESP_ERR_INVALID_ARG);
    PARSER_RETURN_ON_FALSE(data_len >= FRAME_MIN_SIZE, "data_len less than FRAME_MIN_SIZE", ESP_ERR_INVALID_SIZE);

    return parse_into_frame(data, data_len, out_frame);
}

esp_err_t beam_serialize_frame(const beam_frame_t *frame, uint8_t *out_buffer, size_t buffer_size, size_t *out_size)
{
    PARSER_RETURN_ON_FALSE(frame != NULL, "frame pointer is NULL", ESP_ERR_INVALID_ARG);
    PARSER_RETURN_ON_FALSE(out_buffer != NULL, "out_buffer pointer is NULL", ESP_ERR_INVALID_ARG);
    PARSER_RETURN_ON_FALSE(frame->header.len <= MAX_PAYLOAD_SIZE,
                           "frame.header.len exceeds MAX_PAYLOAD_SIZE",
                           ESP_ERR_INVALID_STATE);

    return serialize_frame(frame, out_buffer, buffer_size, out_size);
}
