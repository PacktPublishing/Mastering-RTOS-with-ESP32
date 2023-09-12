/**
 * @file webserver.c
 * @author Bhupinderjeet Singh
 * @brief
 * @version 0.1
 * @date 2023-09-06
 *
 * @copyright Copyright (c) 2023
 *
 * Reference: https://github.com/espressif/esp-idf/blob/62ee4135e033cc85eb0d7572e5b5d147bcb4349e/examples/protocols/http_server/simple/main/main.c
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
#include "nvs_flash.h"
#include "esp_http_server.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "task_wifi.h"
#include "task_blink.h"
#include "utils.h"

// PROJECT DETAILS
/*
	The webserver.c contains functions to service local http webserver feature, where a webpage which is hosted on ESP32 can be accessed from a device in same network
	to check the current Wi-Fi credentials and also new configuration can be updated.
	SECTIONS:
	4.1 Function to handle POST requests for updating WiFi credentials
	4.2 - Get value of expected key from query string
	4.3 - save new wifi configuration into memory
	4.4 - Send notification to wifi task
	4.5 - Resume LED Blink Task to indicate user
	4.6 - Function to handle GET requests for the configuration page
	4.7 - Sample HTML page preparation
	4.8 - Function to get the WiFi connection status

	Note: When new credentials will be provided, the html page will be no more accessible as ESP32 connects to new network and the same IP is no more valid.
*/

// DEFINES
#define TAG "webserver"

// GLOBALS
static httpd_uri_t config_get_uri;
static httpd_uri_t config_post_uri;

// FUNCTION PROTOTYPES
static esp_err_t config_post_handler(httpd_req_t *req);
static esp_err_t config_get_handler(httpd_req_t *req);
static const char *get_wifi_status(void);
static void wifi_credentials_update(const char *ssid, const char *password);

// FUNCTIONS
void webserver_init(void)
{
	config_get_uri.uri = "/config";
	config_get_uri.method = HTTP_GET;
	config_get_uri.handler = config_get_handler;

	config_post_uri.uri = "/config";
	config_post_uri.method = HTTP_POST;
	config_post_uri.handler = config_post_handler;

	httpd_handle_t server = NULL;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	ESP_ERROR_CHECK(httpd_start(&server, &config));

	ESP_ERROR_CHECK(httpd_register_uri_handler(server, &config_get_uri));
	ESP_ERROR_CHECK(httpd_register_uri_handler(server, &config_post_uri));
}

// 4.1 Function to handle POST requests for updating WiFi credentials
static esp_err_t config_post_handler(httpd_req_t *req)
{
	// Extract new SSID and password from the POST request content
	char new_ssid[32];
	char new_password[64];
	char buf[100];
	bool found = false;

	int ret, remaining = req->content_len;
	ESP_LOGI(TAG, "Context length: %d", req->content_len);

	while (remaining > 0)
	{
		/* Read the data for the request */
		if ((ret = httpd_req_recv(req, buf,
								  MIN(remaining, sizeof(buf)))) <= 0)
		{
			if (ret == HTTPD_SOCK_ERR_TIMEOUT)
			{
				/* Retry receiving if timeout occurred */
				continue;
			}
			return ESP_FAIL;
		}

		remaining -= ret;

		/* Log data received */
		ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
		ESP_LOGI(TAG, "%.*s", ret, buf);
		ESP_LOGI(TAG, "====================================");

		/* Clean the buffers */
		memset(new_ssid, 0, sizeof(new_ssid));
		memset(new_password, 0, sizeof(new_password));

		found = true;

		// 4.2 - Get value of expected key from query string
		if (httpd_query_key_value(buf, "ssid", new_ssid, sizeof(new_ssid)) != ESP_OK)
		{
			ESP_LOGE(TAG, "ssid not sent");
			found = false;
		}
		if (httpd_query_key_value(buf, "password", new_password, sizeof(new_password)) != ESP_OK)
		{
			ESP_LOGE(TAG, "password not sent");
			found = false;
		}

		if (found)
		{
			/* Replace '+' with space */
			utils_replace_characters(new_ssid, '+', ' ');

			/* Print on Debug port*/
			ESP_LOGI(TAG, "SSID: %s", new_ssid);
			ESP_LOGI(TAG, "PASS: %s", new_password);

			// 4.3 - save new wifi configuration into memory
			wifi_credentials_update(new_ssid, new_password);

			// 4.4 - Send notification to wifi task
			task_wifi_notify_new_credentials();

			// 4.5 - Resume LED Blink Task to indicate user
			task_blink_resume();
		}
	}

	// Redirect back to the configuration page
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/config");
	httpd_resp_send(req, NULL, 0);
	return ESP_OK;
}

// 4.6 - Function to handle GET requests for the configuration page
static esp_err_t config_get_handler(httpd_req_t *req)
{
	wifi_config_t wifi_config;
	esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_config);

	char response[600];

	// 4.7 - Sample HTML page preparation
	snprintf(response, sizeof(response),
			 "<html><body>"
			 "<h1>WiFi Configuration</h1>"
			 "<p>Current SSID: %s</p>"
			 "<p>Current Password: %s</p>"
			 "<p>Connection Status: %s</p>"
			 "<form action=\"/config\" method=\"post\">"
			 "<label for=\"ssid\">New SSID:</label><br>"
			 "<input type=\"text\" id=\"ssid\" name=\"ssid\"><br><br>"
			 "<label for=\"password\">New Password:</label><br>"
			 "<input type=\"password\" id=\"password\" name=\"password\"><br><br>"
			 "<input type=\"end\" id=\"end\" name=\"end\" value=\"end\" hidden><br><br>"
			 "<input type=\"submit\" value=\"Submit\">"
			 "</form>"
			 "</body></html>",
			 wifi_config.sta.ssid, wifi_config.sta.password, get_wifi_status());

	httpd_resp_send(req, response, strlen(response));
	return ESP_OK;
}

// 4.8 - Function to get the WiFi connection status
static const char *get_wifi_status(void)
{
	wifi_ap_record_t ap_info;
	if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK)
	{
		if (ap_info.rssi >= -50)
		{
			return "Connected (Excellent)";
		}
		else if (ap_info.rssi >= -60)
		{
			return "Connected (Good)";
		}
		else if (ap_info.rssi >= -70)
		{
			return "Connected (Fair)";
		}
		else
		{
			return "Connected (Poor)";
		}
	}
	else
	{
		return "Not Connected";
	}
}

static void wifi_credentials_update(const char *ssid, const char *password)
{
	nvs_handle_t nvs;
	esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Error opening NVS: %s", esp_err_to_name(err));
	}

	err = nvs_set_str(nvs, "ssid", ssid);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Error setting SSID: %s", esp_err_to_name(err));
	}

	err = nvs_set_str(nvs, "password", password);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Error setting password: %s", esp_err_to_name(err));
	}

	err = nvs_commit(nvs);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Error committing NVS: %s", esp_err_to_name(err));
	}

	nvs_close(nvs);
}