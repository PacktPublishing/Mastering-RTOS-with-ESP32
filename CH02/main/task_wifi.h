/**
 * @file task_wifi.h
 * @author Bhupinderjeet Singh
 * @brief
 * @version 0.1
 * @date 2023-09-05
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __TASK_WIFI_H__
#define __TASK_WIFI_H__

#include <stdint.h>

typedef struct task_wifi_s
{
	char *ssid;
	char *pass;
	uint8_t ssid_len;
	uint8_t pass_len;
} task_wifi_t;

void task_wifi_setup(void);
void task_wifi_notify_new_credentials(void);

#endif // __TASK_WIFI_H__