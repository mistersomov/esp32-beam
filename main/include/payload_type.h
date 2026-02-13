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

#ifndef BEAM_PAYLOAD_TYPE_H
#define BEAM_PAYLOAD_TYPE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PAYLOAD_SIZE 200 ///< Maximum size of the data payload in bytes

/**
 * @brief Main telemetry structure (12 bytes: roll, pitch, yaw).
 */
typedef struct __attribute__((packed)) beam_payload_telemetry {
    float roll;  ///< Roll angle
    float pitch; ///< Pitch angle
    float yaw;   ///< Yaw angle
} beam_payload_telemetry_t;

/**
 * @brief Battery status structure (5 bytes)
 */
typedef struct __attribute__((packed)) beam_payload_battery {
    uint16_t voltage_mv; ///< mV
    uint16_t current_ma; ///< mA
    uint8_t percent;     ///< %
} beam_payload_battery_t;

/**
 * @brief Frame payload union: use .telemetry, .battery or .raw[] according to header.msg_id (MSG_ID_*).
 */
typedef union beam_payload {
    beam_payload_telemetry_t telemetry; ///< When msg_id == MSG_ID_TELEMETRY
    beam_payload_battery_t battery;     ///< When msg_id == MSG_ID_BATTERY
    uint8_t raw[MAX_PAYLOAD_SIZE];      ///< Raw bytes; or use typed member above
} beam_payload_t;

#ifdef __cplusplus
}
#endif

#endif /* BEAM_PAYLOAD_TYPE_H */