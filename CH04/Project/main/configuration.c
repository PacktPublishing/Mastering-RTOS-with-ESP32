/**
 * @file configuration.c
 * @author Bhupinderjeet Singh
 * @brief
 * @version 0.1
 * @date 2023-11-04
 *
 * @copyright Copyright (c) 2023
 *
 */
// INCLUDES
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "configuration.h"

// DEFINES
#define TAG "config"

// GLOBALS
static const char *storage_name = "config";

#undef PARAM
#define PARAM(type, key, len, name) \
	static uint8_t name##_array[len + 1] = {0};

P_LIST

#undef PARAM
#define PARAM(type, key, len, name) \
	{#key, #name, type, &name##_array, 0, len},

static params_t p_list[P_LIST_TOTAL] = {P_LIST};

// FUNCTION PROTOTYPES
static bool p_params_write_value(param_t i, void *ptr, uint16_t size);
static bool p_params_read_value(param_t i, void *ptr, uint16_t *size);
static esp_err_t p_params_nvs_save_all(void);
static esp_err_t p_params_nvs_load_all(void);
static void p_params_print_all(void);

// FUNCTIONS
bool configuration_init(void)
{
	p_params_nvs_load_all();

	// p_params_write_int8(TEST1, 3);
	// p_params_write_uint8(TEST2, 6);
	// p_params_write_int16(TEST3, 666);
	// p_params_write_uint16(TEST4, 2666);
	// p_params_write_int32(TEST5, 34666);
	// p_params_write_uint32(TEST6, 4434666);
	// p_params_write_string(TEST7, "Bhupinderjeet Singh", 19);

	p_params_print_all();

	// p_params_nvs_save_all();

	return true;
}

bool configuration_process(void)
{
	return true;
}

int8_t p_params_read_int8(param_t i)
{
	int8_t value = 0;
	uint16_t size = 1;

	if (p_params_read_value(i, &value, &size))
	{
		if (size == 1)
		{
			return value;
		}
	}

	return 0;
}

int16_t p_params_read_int16(param_t i)
{
	int16_t value = 0;
	uint16_t size = 2;

	if (p_params_read_value(i, &value, &size))
	{
		if (size == 2)
		{
			return value;
		}
	}

	return 0;
}

int32_t p_params_read_int32(param_t i)
{
	int32_t value = 0;
	uint16_t size = 4;

	if (p_params_read_value(i, &value, &size))
	{
		if (size == 4)
		{
			return value;
		}
	}

	return 0;
}

uint8_t p_params_read_uint8(param_t i)
{
	uint8_t value = 0;
	uint16_t size = 1;

	if (p_params_read_value(i, &value, &size))
	{
		if (size == 1)
		{
			return value;
		}
	}

	return 0;
}

uint16_t p_params_read_uint16(param_t i)
{
	uint16_t value = 0;
	uint16_t size = 2;

	if (p_params_read_value(i, &value, &size))
	{
		if (size == 2)
		{
			return value;
		}
	}

	return 0;
}

uint32_t p_params_read_uint32(param_t i)
{
	uint32_t value = 0;
	uint16_t size = 4;

	if (p_params_read_value(i, &value, &size))
	{
		if (size == 4)
		{
			return value;
		}
	}

	return 0;
}

bool p_params_read_string(param_t i, char *value, uint16_t *len)
{
	if (NULL == len || 0 == *len || NULL == value || P_LIST_TOTAL <= i)
	{
		ESP_LOGE(TAG, "Params!");
		return false;
	}

	uint16_t size = *len;

	memset(value, 0, *len);

	if (p_params_read_value(i, &value, &size))
	{
		return true;
	}

	return false;
}

bool p_params_write_int8(param_t i, int8_t value)
{
	return p_params_write_value(i, &value, 1);
}

bool p_params_write_int16(param_t i, int16_t value)
{
	return p_params_write_value(i, &value, 2);
}

bool p_params_write_int32(param_t i, int32_t value)
{
	return p_params_write_value(i, &value, 4);
}

bool p_params_write_uint8(param_t i, uint8_t value)
{
	return p_params_write_value(i, &value, 1);
}

bool p_params_write_uint16(param_t i, uint16_t value)
{
	return p_params_write_value(i, &value, 2);
}

bool p_params_write_uint32(param_t i, uint32_t value)
{
	return p_params_write_value(i, &value, 4);
}

bool p_params_write_string(param_t i, char *value, uint16_t size)
{
	return p_params_write_value(i, value, size);
}

static bool p_params_write_value(param_t i, void *ptr, uint16_t size)
{
	if (NULL == ptr || 0 == size || P_LIST_TOTAL <= i || size > p_list[i].array_max)
	{
		ESP_LOGE(TAG, "Write Params");
		return false;
	}

	uint8_t *dstPtr = (uint8_t *)p_list[i].array;

	for (uint8_t j = 0; j < size; j++)
	{
		*dstPtr++ = *(uint8_t *)ptr++;
	}

	*dstPtr++ = '\0';

	p_list[i].array_len = size;

	return true;
}

static bool p_params_read_value(param_t i, void *ptr, uint16_t *size)
{
	if (NULL == ptr || 0 == size || P_LIST_TOTAL <= i || NULL == size || *size < p_list[i].array_len)
	{
		ESP_LOGE(TAG, "Read Params");
		return false;
	}

	uint8_t *srcPtr = (uint8_t *)p_list[i].array;

	for (uint8_t j = 0; j < p_list[i].array_len; j++)
	{
		*(uint8_t *)ptr++ = *(uint8_t *)srcPtr++;
	}

	*size = p_list[i].array_len;

	return true;
}

static esp_err_t p_params_nvs_load_all(void)
{
	nvs_handle_t nvs;
	esp_err_t err = nvs_open(storage_name, NVS_READONLY, &nvs);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Params");
		return err;
	}

	// Load all from memory
	for (uint8_t i = 0; i < P_LIST_TOTAL; i++)
	{
		p_list[i].array_len = p_list[i].array_max;
		// ESP_LOGI(TAG, "Reading %s (%s) length = %02d", p_list[i].name, p_list[i].key, p_list[i].array_len);
		memset(p_list[i].array, 0, p_list[i].array_max);
		err = nvs_get_blob(nvs, p_list[i].key, p_list[i].array, &p_list[i].array_len);
		if (err != ESP_OK)
		{
			ESP_LOGE(TAG, "Load %s not found (%s), error = %s", p_list[i].name, p_list[i].key, esp_err_to_name(err));
			p_list[i].array_len = 0;
		}
	}

	// Close
	nvs_close(nvs);

	return ESP_OK;
}

static esp_err_t p_params_nvs_save_all(void)
{
	nvs_handle_t nvs;
	esp_err_t err = nvs_open(storage_name, NVS_READWRITE, &nvs);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Params");
		return err;
	}

	// Save all in memory
	for (uint8_t i = 0; i < P_LIST_TOTAL; i++)
	{
		err = nvs_set_blob(nvs, p_list[i].key, p_list[i].array, p_list[i].array_len);
		if (err != ESP_OK)
		{
			ESP_LOGE(TAG, "Store %s not saved (%s), error = %s", p_list[i].name, p_list[i].key, esp_err_to_name(err));
		}
		else
		{
			ESP_LOGI(TAG, "Writing %s (%s)", p_list[i].name, p_list[i].key);
		}
	}

	// Commit
	err = nvs_commit(nvs);
	if (err != ESP_OK)
		return err;

	// Close
	nvs_close(nvs);

	return ESP_OK;
}

static void p_params_print_all(void)
{
	// Load all from memory
	for (uint8_t i = 0; i < P_LIST_TOTAL; i++)
	{
		if (p_list[i].type == PARAM_TYPE_INT8)
		{
			ESP_LOGI(TAG, "Print %s (%s) = %02d (%02d, %02d)",
					 p_list[i].name,
					 p_list[i].key,
					 p_params_read_int8(i),
					 p_list[i].array_len,
					 p_list[i].array_max);
		}
		else if (p_list[i].type == PARAM_TYPE_INT16)
		{
			ESP_LOGI(TAG, "Print %s (%s) = %d (%02d, %02d)",
					 p_list[i].name,
					 p_list[i].key,
					 p_params_read_int16(i),
					 p_list[i].array_len,
					 p_list[i].array_max);
		}
		else if (p_list[i].type == PARAM_TYPE_INT32)
		{
			ESP_LOGI(TAG, "Print %s (%s) = %ld (%02d, %02d)",
					 p_list[i].name,
					 p_list[i].key,
					 p_params_read_int32(i),
					 p_list[i].array_len,
					 p_list[i].array_max);
		}
		else if (p_list[i].type == PARAM_TYPE_UINT8)
		{
			ESP_LOGI(TAG, "Print %s (%s) = %02d (%02d, %02d)",
					 p_list[i].name,
					 p_list[i].key,
					 p_params_read_uint8(i),
					 p_list[i].array_len,
					 p_list[i].array_max);
		}
		else if (p_list[i].type == PARAM_TYPE_UINT16)
		{
			ESP_LOGI(TAG, "Print %s (%s) = %d (%02d, %02d)",
					 p_list[i].name,
					 p_list[i].key,
					 p_params_read_uint16(i),
					 p_list[i].array_len,
					 p_list[i].array_max);
		}
		else if (p_list[i].type == PARAM_TYPE_UINT32)
		{
			ESP_LOGI(TAG, "Print %s (%s) = %ld (%02d, %02d)",
					 p_list[i].name,
					 p_list[i].key,
					 p_params_read_uint32(i),
					 p_list[i].array_len,
					 p_list[i].array_max);
		}
		else if (p_list[i].type == PARAM_TYPE_STRING)
		{
			ESP_LOGI(TAG, "Print %s (%s) = %s (%02d, %02d)",
					 p_list[i].name,
					 p_list[i].key,
					 (char *)p_list[i].array,
					 p_list[i].array_len,
					 p_list[i].array_max);
		}
	}
}