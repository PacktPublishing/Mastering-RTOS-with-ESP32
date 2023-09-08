/**
 * @file main.c
 * @author Bhupinderjeet Singh
 * @brief
 * @version 0.1
 * @date 2023-09-06
 *
 * @copyright Copyright (c) 2023
 *
 */
// INCLUDES
#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "webserver.h"
#include "task_wifi.h"
#include "task_blink.h"

// DEFINES
#define TAG "app_main"

// GLOBALS

// FUNCTION PROTOTYPES

// FUNCTIONS
void app_main(void)
{
	// NVS initialization is required
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
	ESP_ERROR_CHECK(err);

	ESP_ERROR_CHECK(esp_netif_init());

	// Initialize Tasks
	task_blink_setup();
	task_wifi_setup();

	webserver_init();

	while (true)
	{
		vTaskDelay(1000);
	}
}
