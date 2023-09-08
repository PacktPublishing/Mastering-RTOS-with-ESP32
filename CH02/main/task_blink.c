/**
 * @file task_blink.c
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "task_blink.h"

// DEFINES
#define TAG "task_blink"
#define BLINK_GPIO CONFIG_BLINK_GPIO

// GLOBALS
static TaskHandle_t xHandleBlink = NULL;
static uint8_t s_led_state = 0;

// FUNCTION PROTOTYPES
static void task_blink_function(void *pvParameters);
static void blink_led(void);
static void configure_led(void);

// FUNCTIONS
/**
 * @brief The task blink function will initialize LED blinking feature.
 * The task will stay suspended at start, when new wifi configuration
 * updated from local webserver then task will be resumes to blink the
 * LED will blink for 5 seconds then it will go back to suspended state.
 */
void task_blink_setup(void)
{
	static uint8_t ucParameterToPass; // static because variable need to be present when function ends
	xTaskCreatePinnedToCore(&task_blink_function, TAG, 4096, &ucParameterToPass, tskIDLE_PRIORITY, &xHandleBlink, 0);
	configASSERT(xHandleBlink);
	task_blink_suspend();
}

bool task_blink_suspend(void)
{
	if (xHandleBlink == NULL)
	{
		return false; // task handler not valid
	}

	vTaskSuspend(xHandleBlink);

	ESP_LOGI(TAG, "Task Suspend!");

	return true; // task suspended
}

bool task_blink_resume(void)
{
	if (xHandleBlink == NULL)
	{
		return false;
	}

	vTaskResume(xHandleBlink);

	ESP_LOGI(TAG, "Task resumed");

	return true;
}

static void task_blink_function(void *pvParameters)
{
	static uint8_t loop_count = 0;

	configure_led();
	while (true)
	{
		ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
		blink_led();
		/* Toggle the LED state */
		s_led_state = !s_led_state;
		vTaskDelay(500 / portTICK_PERIOD_MS);
		loop_count++;

		// The task will suspend after blinking LED for 5 seconds
		// wait for 10 counts as this loop runs at 500 msec delay
		if (loop_count > 10)
		{
			task_blink_suspend();
		}
	}
}

static void blink_led(void)
{
	/* Set the GPIO level according to the state (LOW or HIGH)*/
	gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void)
{
	ESP_LOGI(TAG, "Configured to blink GPIO LED!");
	gpio_reset_pin(BLINK_GPIO);
	/* Set the GPIO as a push/pull output */
	gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}