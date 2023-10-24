#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#define TAG "app_main"

// Define semaphores to control access to shared resources
SemaphoreHandle_t highPrioritySemaphore;
SemaphoreHandle_t lowPrioritySemaphore;

// Shared resource
int sharedResource = 0;

// High-priority task
void HighPriorityTask(void *pvParameters)
{
	while (1)
	{
		xSemaphoreTake(highPrioritySemaphore, portMAX_DELAY);
		sharedResource++;
		ESP_LOGI(TAG, "High-priority task updated the resource. Value: %d", sharedResource);
		xSemaphoreGive(highPrioritySemaphore);
		vTaskDelay(pdMS_TO_TICKS(500)); // Simulate some work
	}
}

// Low-priority task
void LowPriorityTask(void *pvParameters)
{
	while (1)
	{
		xSemaphoreTake(lowPrioritySemaphore, portMAX_DELAY);
		sharedResource--;
		ESP_LOGI(TAG, "->Low-priority task updated the resource. Value: %d", sharedResource);
		xSemaphoreGive(lowPrioritySemaphore);
		vTaskDelay(pdMS_TO_TICKS(1000)); // Simulate some work
	}
}

int app_main(void)
{
	TaskHandle_t xHandle1 = NULL;
	TaskHandle_t xHandle2 = NULL;

	// Create semaphores for high-priority and low-priority tasks
	highPrioritySemaphore = xSemaphoreCreateMutex();
	lowPrioritySemaphore = xSemaphoreCreateMutex();

	// Create sender and receiver tasks
	xTaskCreatePinnedToCore(HighPriorityTask, "HighPriority", (3 * 1024), (void *)1, 1, &xHandle1, 0);
	configASSERT(xHandle1);

	xTaskCreatePinnedToCore(LowPriorityTask, "LowPriority", (3 * 1024), (void *)1, 2, &xHandle2, 0);
	configASSERT(xHandle2);

	while (1)
	{
		// Main loop should never be reached
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	return 0;
}