/**
 * @file task_wifi.c
 * @author Bhupinderjeet Singh
 * @brief
 * @version 0.1
 * @date 2023-09-05
 *
 * @copyright Copyright (c) 2023
 *
 */
// INCLUDES
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "task_wifi.h"

// PROJECT DETAILS
/*
	The wifi.c file implement the functions to manage the wifi connection and credentials.
	SECTIONS:
	3.1 - Default Wi-Fi Credentials (Set using SDK Configuration option or Update the MACROs directly in this file)
	3.2 - WiFi configuration settings
	3.3 - Notification API function
	3.4 - Wi-Fi initialization
	3.5 - Wait for notification
	3.6 - Read new credentials from storage
	3.7 - Wi-Fi Event handler private function
*/

// DEFINES
#define TAG "task_wifi"

// 3.1 - Default Wi-Fi Credentials (Set using SDK Configuration option or Update the MACROs directly in this file)
/* WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY CONFIG_ESP_MAXIMUM_RETRY

// 3.2 - WiFi configuration settings
/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

#if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define EXAMPLE_H2E_IDENTIFIER ""
#elif CONFIG_ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_WPA3_SAE_PWE_BOTH
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

// GLOBALS
static TaskHandle_t xHandleWiFi = NULL;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

static int s_retry_num = 0;
static wifi_config_t wifi_config;

// FUNCTION PROTOTYPES
static void task_wifi_function(void *pvParameters);
static void event_handler(void *arg, esp_event_base_t event_base,
						  int32_t event_id, void *event_data);
static void task_wifi_read_credentials_from_storage(void);
static void task_wifi_update_config(task_wifi_t *credentials);

// FUNCTIONS
void task_wifi_setup(void)
{
	static uint8_t ucParameterToPass; // static because variable need to be present when function ends
	xTaskCreatePinnedToCore(&task_wifi_function, TAG, 4096, &ucParameterToPass, 5, &xHandleWiFi, 1);
	configASSERT(xHandleWiFi);
}

// 3.3 - Notification API function
void task_wifi_notify_new_credentials(void)
{
	// Now notify the task
	xTaskNotify(xHandleWiFi, 1, eSetBits);
	ESP_LOGI(TAG, "WiFi Task Notification sent.");
}

static void task_wifi_function(void *pvParameters)
{
	// 3.4 - Wi-Fi initialization
	ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
	s_wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
														ESP_EVENT_ANY_ID,
														&event_handler,
														NULL,
														&instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
														IP_EVENT_STA_GOT_IP,
														&event_handler,
														NULL,
														&instance_got_ip));

	/* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (password len => 8).
	 * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
	 * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
	 * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
	 */
	wifi_config.sta.threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD;
	wifi_config.sta.sae_pwe_h2e = ESP_WIFI_SAE_MODE;

	memset(&wifi_config.sta.sae_h2e_identifier, 0, sizeof(wifi_config.sta.sae_h2e_identifier));
	memcpy(wifi_config.sta.sae_h2e_identifier, EXAMPLE_H2E_IDENTIFIER, strlen(EXAMPLE_H2E_IDENTIFIER));

	/* Following function, Check if we have credentials in storage otherwise use defaults */
	task_wifi_read_credentials_from_storage();

	/* wifi_config should have updated configurations, lets connect */
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG, "wifi_init_sta finished.");

	while (true)
	{
		// 3.5 - Wait for notification
		BaseType_t notification_value = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (notification_value == 1)
		{
			// 3.6 - Read new credentials from storage
			ESP_LOGI(TAG, "WiFi Task Notified.");

			/* Following function will use stored credentials in storage otherwise defaults */
			task_wifi_read_credentials_from_storage();

			/* wifi_config should have updated configurations, lets connect */
			ESP_ERROR_CHECK(esp_wifi_stop());
			ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
			ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
			ESP_ERROR_CHECK(esp_wifi_start());
		}
	}
}

// 3.7 - Wi-Fi Event handler private function
static void event_handler(void *arg, esp_event_base_t event_base,
						  int32_t event_id, void *event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		esp_wifi_connect();
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
		{
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(TAG, "retry to connect to the AP");
		}
		else
		{
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		ESP_LOGI(TAG, "connect to the AP fail");
	}
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
		ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);

		// Get and print the current SSID
		wifi_ap_record_t ap_info;
		if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK)
		{
			ESP_LOGI(TAG, "Connected to SSID: %s", ap_info.ssid);
		}
	}
}

static void task_wifi_read_credentials_from_storage(void)
{
	task_wifi_t credentials;

	char ssid[32];
	char password[64];
	size_t ssid_len = sizeof(ssid);
	size_t password_len = sizeof(password);

	/* Clean the buffers */
	memset(ssid, 0, sizeof(ssid));
	memset(password, 0, sizeof(password));

	// Read Wi-Fi credentials from NVS
	bool read_wifi_credentials = false;
	nvs_handle_t nvs;
	esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Error opening NVS: %s", esp_err_to_name(err));
	}

	err = nvs_get_str(nvs, "ssid", ssid, &ssid_len);
	if (err == ESP_OK)
	{
		err = nvs_get_str(nvs, "password", password, &password_len);
		if (err == ESP_OK)
		{
			ESP_LOGI(TAG, "Read SSID: %s", ssid);
			ESP_LOGI(TAG, "Read Password: %s", password);
			credentials.ssid = ssid;
			credentials.pass = password;
			credentials.ssid_len = ssid_len;
			credentials.pass_len = password_len;
			read_wifi_credentials = true;
		}
		else
		{
			ESP_LOGE(TAG, "Error reading password: %s", esp_err_to_name(err));
		}
	}
	else
	{
		ESP_LOGE(TAG, "Error reading ssid: %s", esp_err_to_name(err));
	}

	if (!read_wifi_credentials)
	{
		ESP_LOGI(TAG, "Using Default SSID (%s) and Password (%s)",
				 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);

		credentials.ssid = EXAMPLE_ESP_WIFI_SSID;
		credentials.pass = EXAMPLE_ESP_WIFI_PASS;
		credentials.ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID);
		credentials.pass_len = strlen(EXAMPLE_ESP_WIFI_PASS);
	}

	task_wifi_update_config(&credentials);
}

static void task_wifi_update_config(task_wifi_t *credentials)
{
	if (credentials == NULL ||
		credentials->ssid == NULL ||
		credentials->pass == NULL ||
		credentials->ssid_len == 0 ||
		credentials->pass_len == 0)
	{
		ESP_LOGE(TAG, "Wrong params, returning");
		return;
	}

	// clean destination
	memset(wifi_config.sta.ssid, 0, sizeof(wifi_config.sta.ssid));
	memset(wifi_config.sta.password, 0, sizeof(wifi_config.sta.password));

	// copy data
	memcpy(wifi_config.sta.ssid, credentials->ssid, credentials->ssid_len);
	memcpy(wifi_config.sta.password, credentials->pass, credentials->pass_len);

	ESP_LOGI(TAG, "Credentials updated in wifi_config");
}