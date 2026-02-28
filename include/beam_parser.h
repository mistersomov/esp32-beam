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

#ifndef BEAM_PARSER_H
#define BEAM_PARSER_H

#include "beam_frame.h"
#include "esp_err.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Parses a raw buffer into frame.
 *
 * @param data Raw byte array from esp_now_recv_cb.
 * @param data_len Length of the received data.
 * @param out_frame Pointer to the frame to fill if valid.
 *
 * @return ESP_OK if the frame is valid.
 *         ESP_ERR_INVALID_ARG if data or out_frame is NULL.
 *         ESP_ERR_INVALID_SIZE if data_len is too short or payload length is invalid.
 *         ESP_ERR_INVALID_CRC if the CRC does not match.
 *
 * @note Use FRAME_MIN_SIZE for minimum buffer length.
 */
esp_err_t beam_parse_into_frame(const uint8_t *data, size_t data_len, beam_frame_t *out_frame);

/**
 * @brief Serializes a frame into raw buffer (header + payload + CRC).
 *
 * @param frame Pointer to the frame to serialize. Must not be NULL.
 * @param out_buffer Buffer to write serialized frame. Must not be NULL.
 * @param buffer_size Size of out_buffer in bytes.
 * @param[out] out_size Optional pointer to receive the number of bytes written. Can be NULL.
 *
 * @return ESP_OK on success.
 *         ESP_ERR_INVALID_ARG if frame or out_buffer is NULL.
 *         ESP_ERR_INVALID_SIZE if buffer_size is too small for the frame.
 *         ESP_ERR_INVALID_STATE if frame.header.len exceeds MAX_PAYLOAD_SIZE.
 */
esp_err_t beam_serialize_frame(const beam_frame_t *frame, uint8_t *out_buffer, size_t buffer_size, size_t *out_size);

#ifdef __cplusplus
}
#endif

#endif /* BEAM_PARSER_H */