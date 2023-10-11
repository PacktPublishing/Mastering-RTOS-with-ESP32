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
#include "adc_read.h"

// PROJECT DETAILS
/*
	This project demonstrate practical use of RTOS APIs such as task creation, suspend, resume and notify one task from another.
	The example project is built to also solve real world problem where in IoT projects, most of the time new configuration needs
	to be updated wirelessly. In this example we are starting a http server which can be accessed from http://IP.local from any
	device in same network where the IP will be printed on debug port when connected to predefined Wi-Fi credentials.
	SECTIONS:
	1. - main.c
	2. - task_blink.c
	3. - task_wifi.c
	4. - utils.c
	5. - webserver.c

	Each file will have further sections for understanding the code implementation.
*/

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

	// Required to initialize net-work foundation
	ESP_ERROR_CHECK(esp_netif_init());

	// Initialize Tasks
	task_blink_setup();
	task_wifi_setup();

	// Initialize modules
	webserver_init();

	adc_read_setup();

	// Infinite loop (Ideal Task Loop)
	while (true)
	{
		// You can add your function calls here
		vTaskDelay(1000);
	}
}
