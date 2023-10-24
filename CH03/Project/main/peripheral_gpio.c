/**
 * @file peripheral_gpio.c
 * @author Bhupinderjeet Singh
 * @brief
 * @version 0.1
 * @date 2023-10-11
 *
 * @copyright Copyright (c) 2023
 *
 */

/**
 * Brief:
 * This test code shows how to configure gpio and how to use gpio interrupt.
 *
 * GPIO status:
 * GPIO4:  input, pulled up, interrupt from rising edge and falling edge
 * GPIO5:  input, pulled up, interrupt from rising edge.
 *
 * Note. These are the default GPIO pins to be used in the example. You can
 * change IO pins in menuconfig.
 *
 * Test:
 * Connect GPIO18(8) with GPIO4
 * Connect GPIO19(9) with GPIO5
 * Generate pulses on GPIO18(8)/19(9), that triggers interrupt on GPIO4/5
 *
 */
/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

// INCLUDES
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <inttypes.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

// DEFINES
#define TAG "GPIO"
#define GPIO_OUTPUT_IO_0 0
#define GPIO_OUTPUT_IO_1 4
#define GPIO_OUTPUT_PIN_SEL ((1ULL << GPIO_OUTPUT_IO_0) | (1ULL << GPIO_OUTPUT_IO_1))
/*
 * Let's say, GPIO_OUTPUT_IO_0=18, GPIO_OUTPUT_IO_1=19
 * In binary representation,
 * 1ULL<<GPIO_OUTPUT_IO_0 is equal to 0000000000000000000001000000000000000000 and
 * 1ULL<<GPIO_OUTPUT_IO_1 is equal to 0000000000000000000010000000000000000000
 * GPIO_OUTPUT_PIN_SEL                0000000000000000000011000000000000000000
 * */
#define GPIO_INPUT_IO_0 18
#define GPIO_INPUT_IO_1 19
#define GPIO_INPUT_PIN_SEL ((1ULL << GPIO_INPUT_IO_0) | (1ULL << GPIO_INPUT_IO_1))
/*
 * Let's say, GPIO_INPUT_IO_0=4, GPIO_INPUT_IO_1=5
 * In binary representation,
 * 1ULL<<GPIO_INPUT_IO_0 is equal to 0000000000000000000000000000000000010000 and
 * 1ULL<<GPIO_INPUT_IO_1 is equal to 0000000000000000000000000000000000100000
 * GPIO_INPUT_PIN_SEL                0000000000000000000000000000000000110000
 * */
#define ESP_INTR_FLAG_DEFAULT 0

// GLOBALS
static QueueHandle_t gpio_evt_queue = NULL;

// FUNCTION PROTOTYPES
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
	uint32_t gpio_num = (uint32_t)arg;
	xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

// FUNCTIONS
void peripheral_gpio_setup(void)
{
	// zero-initialize the config structure.
	gpio_config_t io_conf = {};
	// disable interrupt
	io_conf.intr_type = GPIO_INTR_DISABLE;
	// set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	// bit mask of the pins that you want to set
	io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
	// disable pull-down mode
	io_conf.pull_down_en = 0;
	// disable pull-up mode
	io_conf.pull_up_en = 0;
	// configure GPIO with the given settings
	gpio_config(&io_conf);

	// interrupt of rising edge
	io_conf.intr_type = GPIO_INTR_POSEDGE;
	// bit mask of the pins, use GPIO4/5 here
	io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
	// set as input mode
	io_conf.mode = GPIO_MODE_INPUT;
	// enable pull-up mode
	io_conf.pull_up_en = 1;
	gpio_config(&io_conf);

	// change gpio interrupt type for one pin
	gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);

	// create a queue to handle gpio event from isr
	gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

	// install gpio isr service
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	// hook isr handler for specific gpio pin
	gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void *)GPIO_INPUT_IO_0);
	// hook isr handler for specific gpio pin
	gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void *)GPIO_INPUT_IO_1);

	// // remove isr handler for gpio number.
	// gpio_isr_handler_remove(GPIO_INPUT_IO_0);
	// // hook isr handler for specific gpio pin again
	// gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void *)GPIO_INPUT_IO_0);

	ESP_LOGI(TAG, "Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
}

void peripheral_gpio_loop(void)
{
	uint32_t io_num;

	if (xQueueReceive(gpio_evt_queue, &io_num, pdMS_TO_TICKS(100)))
	{
		ESP_LOGI(TAG, "GPIO[%" PRIu32 "] intr, val: %d\n", io_num, gpio_get_level(io_num));
	}
}

void peripheral_gpio_on(void)
{
	gpio_set_level(GPIO_OUTPUT_IO_0, 1);
	gpio_set_level(GPIO_OUTPUT_IO_1, 1);
}

void peripheral_gpio_off(void)
{
	gpio_set_level(GPIO_OUTPUT_IO_0, 0);
	gpio_set_level(GPIO_OUTPUT_IO_1, 0);
}