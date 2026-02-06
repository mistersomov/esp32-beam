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

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "esp_adc/adc_cali.h"
#include "esp_err.h"
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AXES_COUNT CONFIG_AXES_COUNT

/**
 * @brief Controller direction enumeration
 */
typedef enum direction {
    FORWARD,  ///< Forward direction
    BACKWARD, ///< Backward direction
    LEFT,     ///< Left direction
    RIGHT     ///< Right direction
} direction_t;

/**
 * @brief ADC operation mode enumeration
 */
typedef enum adc_mode {
    ONE_SHOT,  ///< One-shot ADC reading mode
    CONTINUOUS ///< Continuous ADC reading mode
} adc_mode_t;

/**
 * @brief ADC block configuration structure
 */
typedef struct adc_block {
    adc_unit_t unit_id;             ///< ADC unit identifier
    adc_channel_t channel;          ///< ADC channel number
    adc_atten_t atten;              ///< ADC attenuation setting
    adc_bitwidth_t bitwidth;        ///< ADC bit width
    adc_mode_t mode;                ///< ADC operation mode
    adc_cali_handle_t cali_handler; ///< ADC calibration handler
} adc_block_t;

/**
 * @brief Controller axis unit structure
 *
 * Represents a single axis configuration
 */
typedef struct controller_axis_unit {
    adc_block_t adc_block; ///< ADC block configuration
    direction_t direction; ///< Direction mapping for this axis
} controller_axis_unit_t;

/**
 * @brief Controller configuration structure
 *
 * Contains configuration for all controller axes.
 */
typedef struct controller_config {
    controller_axis_unit_t axes[AXES_COUNT]; ///< Array of axis configurations
} controller_config_t;

/**
 * @brief Configure the controller with the provided configuration
 *
 * This function initializes ADC drivers and calibration for all configured axes.
 * It sets up one-shot or continuous ADC modes as specified in the configuration.
 *
 * @param[in] cfg Pointer to controller configuration structure. Must not be NULL.
 *
 * @return
 *      - ESP_OK: Configuration successful
 *      - ESP_ERR_INVALID_ARG: Invalid configuration parameter
 *      - ESP_ERR_NO_MEM: Memory allocation failed
 *      - ESP_FAIL: Configuration failed
 */
esp_err_t controller_init(const controller_config_t *cfg);

/**
 * @brief Delete the controller
 *
 * @param[in] cfg Pointer to controller configuration structure. Must not be NULL.
 *
 * @return
 *        - ESP_OK:              On success
 *        - ESP_ERR_INVALID_ARG: Invalid arguments
 *        - ESP_ERR_NOT_FOUND:   The ADC peripheral to be disclaimed isn't in use
 */
esp_err_t controller_deinit(const controller_config_t *cfg);

/**
 * @brief Read ADC values from all configured controller axes
 *
 * This function reads raw ADC values and converts them to voltage using
 * calibration if available. Results are logged via ESP_LOG.
 *
 * @param[in] cfg Pointer to controller configuration structure. Must not be NULL.
 *                The configuration must have been previously initialized with
 *                controller_init().
 * @return
 *      - ESP_OK: Configuration successful
 *      - ESP_ERR_INVALID_ARG: Invalid configuration parameter
 *      - ESP_ERR_NO_MEM: Memory allocation failed
 *      - ESP_FAIL: Configuration failed
 */
esp_err_t controller_read_values(const controller_config_t *cfg);

#ifdef __cplusplus
}
#endif

#endif /* CONTROLLER_H */