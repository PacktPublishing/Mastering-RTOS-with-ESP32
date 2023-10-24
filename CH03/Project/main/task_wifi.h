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

bool is_wifi_connected(void);

bool task_wifi_update_credentials(char *ssid, uint16_t ssid_len, char *pass, uint16_t pass_len);
bool task_wifi_get_credentials(char *ssid, uint16_t ssid_len_max, char *pass, uint16_t pass_len_max);

#endif // __TASK_WIFI_H__