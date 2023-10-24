/**
 * @file adc_read.cpp
 * @author Bhupinderjeet Singh
 * @brief
 * @version 0.1
 * @date 2023-10-11
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

// DEFINES
#define TAG "ADC"
#define EXAMPLE_ADC1_CHAN0 ADC_CHANNEL_6 // GPIO 34
#define EXAMPLE_ADC1_CHAN1 ADC_CHANNEL_7 // GPIO 35
#define EXAMPLE_ADC_ATTEN ADC_ATTEN_DB_11

// GLOBALS
adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t adc1_cali_chan0_handle = NULL;
adc_cali_handle_t adc1_cali_chan1_handle = NULL;
bool do_calibration1_chan0 = false;
bool do_calibration1_chan1 = false;

static int adc_raw[2][10];
static int voltage[2][10];

// FUNCTION PROTOTYPES
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);

// FUNCTIONS
void peripheral_adc_setup(void)
{
	adc_oneshot_unit_init_cfg_t init_config1 = {
		.unit_id = ADC_UNIT_1,
	};
	ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

	adc_oneshot_chan_cfg_t config = {
		.bitwidth = ADC_BITWIDTH_DEFAULT,
		.atten = EXAMPLE_ADC_ATTEN,
	};
	ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN0, &config));
	ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN1, &config));

	do_calibration1_chan0 = adc_calibration_init(ADC_UNIT_1, EXAMPLE_ADC1_CHAN0, EXAMPLE_ADC_ATTEN, &adc1_cali_chan0_handle);
	do_calibration1_chan1 = adc_calibration_init(ADC_UNIT_1, EXAMPLE_ADC1_CHAN1, EXAMPLE_ADC_ATTEN, &adc1_cali_chan1_handle);
}

int peripheral_adc_value_io_34(void)
{
	ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN0, &adc_raw[0][0]));
	ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, EXAMPLE_ADC1_CHAN0, adc_raw[0][0]);

	if (do_calibration1_chan0)
	{
		ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan0_handle, adc_raw[0][0], &voltage[0][0]));
		ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, EXAMPLE_ADC1_CHAN0, voltage[0][0]);
	}

	return voltage[0][0];
}

int peripheral_adc_value_io_35(void)
{
	ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN1, &adc_raw[0][1]));
	ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, EXAMPLE_ADC1_CHAN1, adc_raw[0][1]);
	if (do_calibration1_chan1)
	{
		ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan1_handle, adc_raw[0][1], &voltage[0][1]));
		ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, EXAMPLE_ADC1_CHAN1, voltage[0][1]);
	}

	return voltage[0][1];
}

static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
	adc_cali_handle_t handle = NULL;
	esp_err_t ret = ESP_FAIL;
	bool calibrated = false;

	if (!calibrated)
	{
		ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
		adc_cali_line_fitting_config_t cali_config = {
			.unit_id = unit,
			.atten = atten,
			.bitwidth = ADC_BITWIDTH_DEFAULT,
		};
		ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
		if (ret == ESP_OK)
		{
			calibrated = true;
		}
	}

	*out_handle = handle;
	if (ret == ESP_OK)
	{
		ESP_LOGI(TAG, "Calibration Success");
	}
	else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated)
	{
		ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
	}
	else
	{
		ESP_LOGE(TAG, "Invalid arg or no memory");
	}

	return calibrated;
}