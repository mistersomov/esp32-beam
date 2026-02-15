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

#ifndef BEAM_MESSAGE_COMMON_H
#define BEAM_MESSAGE_COMMON_H

#include "esp_bit_defs.h"
#include <stdint.h>

#define MSG_TYPE_MASK (0x03U)
#define MSG_TYPE_INFO (0x00U)
#define MSG_TYPE_COMMAND (0x01U)
#define MSG_TYPE_STATUS (0x02U)

#define BEAM_BIT(n) BIT(n)

typedef uint8_t beam_flags_t;

#define MSG_FLAG_PRIORITY BEAM_BIT(0)
#define MSG_FLAG_ACK_REQ BEAM_BIT(1)

typedef uint8_t beam_msg_category_t;
typedef enum beam_message_category {
    MSG_CAT_TELEMETRY, ///< Orientation data
    MSG_CAT_BATTERY,   ///< Battery data
} beam_message_category_t;

#endif /* BEAM_MESSAGE_COMMON_H */