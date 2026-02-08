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

#include "esp_err.h"
#include "frame.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Minimum frame size in bytes (header 3 + payload 0 + CRC 2). */
#define BEAM_FRAME_MIN_SIZE 5

/**
 * @brief Validates a raw buffer received from ESP-NOW.
 *
 * @param data Raw byte array from esp_now_recv_cb.
 * @param data_len Length of the received data.
 * @param out_frame Pointer to the frame to fill if valid.
 *
 * @return ESP_OK if the frame is valid and out_frame was filled.
 *         ESP_ERR_INVALID_ARG if data or out_frame is NULL.
 *         ESP_ERR_INVALID_SIZE if data_len is too short or payload length is invalid.
 *         ESP_ERR_INVALID_CRC if the CRC does not match.
 * @note Use BEAM_FRAME_MIN_SIZE for minimum buffer length.
 */
esp_err_t beam_validate_frame(const uint8_t *data, size_t data_len, beam_frame_t *out_frame);

#ifdef __cplusplus
}
#endif

#endif /* BEAM_PARSER_H */