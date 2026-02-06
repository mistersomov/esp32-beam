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

#include "controller.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_check.h"
#include "esp_log.h"

static const char *TAG = "CONTROLLER";
#define CONTROLLER_RETURN_ON_FALSE(func, str, ret_val) ESP_RETURN_ON_FALSE(func, ret_val, TAG, "%s", str)
#define CONTROLLER_CHECK(func) ESP_ERROR_CHECK(func)

#define SAMPLES_PER_AXIS CONFIG_SAMPLES_PER_AXIS

static int adc_raw[AXES_COUNT][SAMPLES_PER_AXIS];
static int voltage[AXES_COUNT][SAMPLES_PER_AXIS];
static adc_oneshot_unit_handle_t adc_one_shot_handlers[2] = {NULL, NULL};

static esp_err_t configure_one_shot_driver(const controller_axis_unit_t *axis);
static bool adc_calibration_init(adc_block_t *adc_block);
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
static esp_err_t configure_curve_fitting_calibration(adc_block_t *adc_block, adc_cali_handle_t *handler);
#endif
#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
static esp_err_t configure_line_fitting_calibration(adc_block_t *adc_block, adc_cali_handle_t *handler);
#endif

esp_err_t controller_config(const controller_config_t *cfg)
{
    CONTROLLER_RETURN_ON_FALSE(cfg, "Controller config pointer error", ESP_ERR_INVALID_ARG);

    for (int i = 0; i != AXES_COUNT; ++i) {
        controller_axis_unit_t *axis = &cfg->axes[i];

        if (axis->adc_block.mode == CONTINUOUS) {
        }
        else {
            CONTROLLER_CHECK(configure_one_shot_driver(axis));
        }
        adc_calibration_init(&axis->adc_block);
    }

    return ESP_OK;
}

esp_err_t controller_read_values(const controller_config_t *cfg)
{
    CONTROLLER_RETURN_ON_FALSE(cfg, "Controller config pointer error", ESP_ERR_INVALID_ARG);

    for (int i = 0; i != AXES_COUNT; ++i) {
        const controller_axis_unit_t *axis = &cfg->axes[i];
        CONTROLLER_RETURN_ON_FALSE(axis, "Axis pointer error", ESP_ERR_INVALID_ARG);

        int unit_idx = (axis->adc_block.unit_id == ADC_UNIT_1) ? 0 : 1;
        adc_cali_handle_t cali_handler = axis->adc_block.cali_handler;
        CONTROLLER_RETURN_ON_FALSE(cali_handler, "Calibration handler error", ESP_ERR_INVALID_ARG);

        CONTROLLER_CHECK(adc_oneshot_read(adc_one_shot_handlers[unit_idx], axis->adc_block.channel, adc_raw[i]));
        if (cali_handler) {
            CONTROLLER_CHECK(adc_cali_raw_to_voltage(cali_handler, adc_raw[i][0], voltage[i]));
            ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", unit_idx, axis->adc_block.channel, voltage[i][0]);
        }
    }

    return ESP_OK;
}

esp_err_t configure_one_shot_driver(const controller_axis_unit_t *axis)
{
    CONTROLLER_RETURN_ON_FALSE(axis, "Axis pointer error", ESP_ERR_INVALID_ARG);

    int unit_idx = (axis->adc_block.unit_id == ADC_UNIT_1) ? 0 : 1;
    if (adc_one_shot_handlers[unit_idx] == NULL) {
        adc_oneshot_unit_init_cfg_t unit_config = {
            .unit_id = axis->adc_block.unit_id,
        };
        CONTROLLER_CHECK(adc_oneshot_new_unit(&unit_config, &adc_one_shot_handlers[unit_idx]));
    }

    adc_oneshot_chan_cfg_t channel_config = {
        .atten = axis->adc_block.atten,
        .bitwidth = axis->adc_block.bitwidth,
    };

    return adc_oneshot_config_channel(adc_one_shot_handlers[unit_idx], axis->adc_block.channel, &channel_config);
}

bool adc_calibration_init(adc_block_t *adc_block)
{
    CONTROLLER_RETURN_ON_FALSE(adc_block, "ADC block error", ESP_ERR_INVALID_ARG);

    adc_cali_handle_t handle = NULL;
    bool calibrated = false;
    esp_err_t ret = ESP_FAIL;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ret = configure_curve_fitting_calibration(adc_block, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ret = configure_line_fitting_calibration(adc_block, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    adc_block->cali_handler = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    }
    else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    }
    else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
esp_err_t configure_curve_fitting_calibration(adc_block_t *adc_block, adc_cali_handle_t *handler)
{
    CONTROLLER_RETURN_ON_FALSE(adc_block, "ADC block error", ESP_ERR_INVALID_ARG);
    CONTROLLER_RETURN_ON_FALSE(handler, "ADC calibration handler error", ESP_ERR_INVALID_ARG);

    ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = adc_block->unit_id,
        .chan = adc_block->channel,
        .atten = adc_block->atten,
        .bitwidth = adc_block->bitwidth,
    };

    return adc_cali_create_scheme_curve_fitting(&cali_config, handler);
}
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
esp_err_t configure_line_fitting_calibration(adc_block_t *adc_block, adc_cali_handle_t *handler)
{
    CONTROLLER_RETURN_ON_FALSE(adc_block, "ADC block error", ESP_ERR_INVALID_ARG);
    CONTROLLER_RETURN_ON_FALSE(handler, "ADC calibration handler error", ESP_ERR_INVALID_ARG);

    ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = adc_block->unit_id,
        .atten = adc_block->atten,
        .bitwidth = adc_block->bitwidth,
    };

    return adc_cali_create_scheme_line_fitting(&cali_config, handler);
}
#endif
