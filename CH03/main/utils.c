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
