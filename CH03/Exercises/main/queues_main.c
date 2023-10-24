#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

// Reference: https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32/api-reference/system/freertos.html#queue-api

#define TAG "app_main"

// Define the queue handle
QueueHandle_t messageQueue;

struct AMessage
{
	char ucMessageID;
	char ucData[30];
} xMessage;

// Task that sends a message to the queue
void SenderTask(void *pvParameters)
{
	struct AMessage message;

	while (1)
	{
		// Clear the ucData memory
		memset(message.ucData, 0, sizeof(message.ucData));

		// Increment the ID
		message.ucMessageID += 1;

		if (message.ucMessageID > 99)
		{
			message.ucMessageID = 0;
		}

		// Write the data
		snprintf(message.ucData, sizeof(message.ucData), "Hello, Receiver!(%02d)", message.ucMessageID);

		ESP_LOGI(TAG, "Sending: ID=%d, Message=%s", message.ucMessageID, message.ucData);
		xQueueSend(messageQueue, &message, portMAX_DELAY);
		vTaskDelay(pdMS_TO_TICKS(1000)); // Send a message every 1 second
	}
}

// Task that receives and processes messages from the queue
void ReceiverTask(void *pvParameters)
{
	struct AMessage pxMessage;

	while (1)
	{
		if (xQueueReceive(messageQueue, &pxMessage, portMAX_DELAY) == pdTRUE)
		{
			ESP_LOGI(TAG, "Received: ID=%d, Message=%s", pxMessage.ucMessageID, pxMessage.ucData);
		}
	}
}

int app_main(void)
{
	TaskHandle_t xHandle1 = NULL;
	TaskHandle_t xHandle2 = NULL;

	// Create a queue to hold messages
	messageQueue = xQueueCreate(5, sizeof(xMessage));

	// Create sender and receiver tasks
	xTaskCreatePinnedToCore(SenderTask, "Sender", (3 * 1024), (void *)1, tskIDLE_PRIORITY, &xHandle1, 0);
	configASSERT(xHandle1);

	xTaskCreatePinnedToCore(ReceiverTask, "Receiver", (3 * 1024), (void *)1, tskIDLE_PRIORITY, &xHandle2, 0);
	configASSERT(xHandle2);

	while (1)
	{
		// Main loop should never be reached
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	return 0;
}