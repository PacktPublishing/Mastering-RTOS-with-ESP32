#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"

// Reference:

#define TAG "app_main"

// Define a Mutex handle
SemaphoreHandle_t resourceMutex;

// Shared resource
int sharedResource = 0;

// Task that modifies the shared resource
void Task1(void *pvParameters)
{
	while (1)
	{
		// Attempt to take the Mutex to access the critical section
		if (xSemaphoreTake(resourceMutex, portMAX_DELAY) == pdTRUE)
		{
			// Modify the shared resource
			sharedResource++;
			ESP_LOGI(TAG, "Task1 updated sharedResource: %d", sharedResource);

			// Release the Mutex to allow other tasks to access the resource
			xSemaphoreGive(resourceMutex);
		}

		vTaskDelay(pdMS_TO_TICKS(500)); // Simulate some work
	}
}

// Another task that also modifies the shared resource
void Task2(void *pvParameters)
{
	while (1)
	{
		// Attempt to take the Mutex to access the critical section
		if (xSemaphoreTake(resourceMutex, portMAX_DELAY) == pdTRUE)
		{
			// Modify the shared resource
			sharedResource--;
			ESP_LOGI(TAG, "Task2 updated sharedResource: %d\n", sharedResource);

			// Release the Mutex to allow other tasks to access the resource
			xSemaphoreGive(resourceMutex);
		}

		vTaskDelay(pdMS_TO_TICKS(500)); // Simulate some work
	}
}

int app_main(void)
{
	TaskHandle_t xHandle1 = NULL;
	TaskHandle_t xHandle2 = NULL;

	// Create a Mutex to protect the critical section
	resourceMutex = xSemaphoreCreateMutex();

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