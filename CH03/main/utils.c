/**
 * @file utils.c
 * @author Bhupinderjeet Singh
 * @brief
 * @version 0.1
 * @date 2023-09-05
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
#include "cJSON.h"

// DEFINES

// GLOBALS

// FUNCTION PROTOTYPES

// FUNCTIONs
bool utils_replace_characters(char *s, const char ch, const char r)
{
	uint16_t i = 0;

	if (s == NULL)
	{
		return false;
	}

	while (*(s + i) != '\0')
	{
		if (*(s + i) == ch)
		{
			*(s + i) = r;
		}
		i++;
	}

	return true;
}

bool findStringValueForKey(const char *jsonString, const char *key, char *value, size_t bufferSize)
{
	// Parse the JSON string
	cJSON *root = cJSON_Parse(jsonString);
	if (root == NULL)
	{
		return false; // Failed to parse JSON
	}

	cJSON *item = cJSON_GetObjectItemCaseSensitive(root, key);

	if (item && cJSON_IsString(item) && item->valuestring)
	{
		if (NULL != value)
		{
			snprintf(value, bufferSize, "%s", item->valuestring);
		}
		cJSON_Delete(root);
		return true;
	}

	cJSON_Delete(root);
	return false; // Key not found or unsupported key type
}
