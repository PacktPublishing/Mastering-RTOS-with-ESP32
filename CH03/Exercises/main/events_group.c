#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_types.h"
#include "esp_err.h"
#include "esp_event.h"

// Reference:

#define TAG "app_main"

// Define an Event Group handle
EventGroupHandle_t eventGroup;

// Define event bit masks
#define EVENT_BIT_TASK1 (1 << 0)

// Task 1: Set the event
void Task1(void *pvParameters)
{
	while (true)
	{
		// Set the event
		xEventGroupSetBits(eventGroup, EVENT_BIT_TASK1);
		vTaskDelay(pdMS_TO_TICKS(1000)); // Simulate some work

		// Clear the event
		xEventGroupClearBits(eventGroup, EVENT_BIT_TASK1);
		vTaskDelay(pdMS_TO_TICKS(1000)); // Simulate some work
	}
}

// Task 2: Wait for the event
void Task2(void *pvParameters)
{
	EventBits_t uxBits;

	while (true)
	{
		// Wait for the event to be set
		uxBits = xEventGroupWaitBits(eventGroup, EVENT_BIT_TASK1, pdTRUE, pdTRUE, portMAX_DELAY);

		if ((uxBits & EVENT_BIT_TASK1) == EVENT_BIT_TASK1)
		{
			// Event is set, perform the desired action
			ESP_LOGI(TAG, "Task 2: Event occurred, performing the action.");
		}

		vTaskDelay(pdMS_TO_TICKS(1000)); // Simulate some work
	}

	vTaskDelete(NULL);
}

int app_main(void)
{
	TaskHandle_t xHandle1 = NULL;
	TaskHandle_t xHandle2 = NULL;

	// Create the Event Group
	eventGroup = xEventGroupCreate();

	// Create sender and receiver tasks
	xTaskCreatePinnedToCore(Task1, "Task1", (3 * 1024), (void *)1, tskIDLE_PRIORITY, &xHandle1, 0);
	configASSERT(xHandle1);

	xTaskCreatePinnedToCore(Task2, "Task2", (3 * 1024), (void *)1, tskIDLE_PRIORITY, &xHandle2, 0);
	configASSERT(xHandle2);

	while (1)
	{
		// Main loop should never be reached
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	return 0;
}