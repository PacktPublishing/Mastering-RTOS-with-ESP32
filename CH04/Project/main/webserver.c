/**
 * @file webserver.c
 * @author Bhupinderjeet Singh
 * @brief
 * @version 0.1
 * @date 2023-09-06
 *
 * @copyright Copyright (c) 2023
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/spiffs.html
 * https://github.com/espressif/esp-idf/tree/272b4091f1/examples/storage/spiffs
 *
 * https://learn.sparkfun.com/tutorials/esp32-relay-web-server/all
 * https://bootswatch.com/solar/#top
 *
 * https://github.com/espressif/esp-idf/blob/272b4091f1f1ff169c84a4ee6b67ded4a005a8a7/examples/storage/spiffsgen/main/CMakeLists.txt
 *
 * https://esp32tutorials.com/esp32-esp-idf-websocket-web-server/
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
#include "webserver.h"
#include "task_wifi.h"
#include "peripheral_adc.h"
#include "peripheral_gpio.h"

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

#define ADC_RESOLUTION 12						  // ADC resolution in bits
#define ADC_MAX_VALUE ((1 << ADC_RESOLUTION) - 1) // Maximum ADC value
#define V_REF 3300								  // Reference voltage in millivolts (3.3V)

// GLOBALS
static httpd_uri_t index_get_uri;
static httpd_uri_t script_get_uri;
static httpd_uri_t gauge_get_uri;
static httpd_uri_t moment_get_uri;
static httpd_uri_t range_get_uri;
static httpd_uri_t style_get_uri;
static httpd_uri_t custom_get_uri;
static httpd_uri_t favicon_get_uri;
static httpd_uri_t fonts_get_uri;
static httpd_uri_t config_post_uri;
static httpd_uri_t config_get_uri;

httpd_handle_t server = NULL;

// FUNCTION PROTOTYPES
static esp_err_t index_get_handler(httpd_req_t *req);
static esp_err_t script_get_handler(httpd_req_t *req);
static esp_err_t gauge_get_handler(httpd_req_t *req);
static esp_err_t moment_get_handler(httpd_req_t *req);
static esp_err_t range_get_handler(httpd_req_t *req);
static esp_err_t style_get_handler(httpd_req_t *req);
static esp_err_t custom_get_handler(httpd_req_t *req);
static esp_err_t fonts_get_handler(httpd_req_t *req);
static esp_err_t favicon_get_handler(httpd_req_t *req);
static esp_err_t config_post_handler(httpd_req_t *req);
static esp_err_t config_get_handler(httpd_req_t *req);
static const char *get_wifi_status(void);

// FUNCTIONS
void webserver_init(void)
{
	index_get_uri.uri = "/";
	index_get_uri.method = HTTP_GET;
	index_get_uri.handler = index_get_handler;

	script_get_uri.uri = "/bootstrap/scripts.js";
	script_get_uri.method = HTTP_GET;
	script_get_uri.handler = script_get_handler;

	gauge_get_uri.uri = "/bootstrap/gauge.min.js";
	gauge_get_uri.method = HTTP_GET;
	gauge_get_uri.handler = gauge_get_handler;

	moment_get_uri.uri = "/bootstrap/moment.min.js";
	moment_get_uri.method = HTTP_GET;
	moment_get_uri.handler = moment_get_handler;

	range_get_uri.uri = "/bootstrap/range.min.js";
	range_get_uri.method = HTTP_GET;
	range_get_uri.handler = range_get_handler;

	style_get_uri.uri = "/bootstrap/bootstrap.min.css";
	style_get_uri.method = HTTP_GET;
	style_get_uri.handler = style_get_handler;

	custom_get_uri.uri = "/bootstrap/custom.min.css";
	custom_get_uri.method = HTTP_GET;
	custom_get_uri.handler = custom_get_handler;

	fonts_get_uri.uri = "/bootstrap/fonts.css";
	fonts_get_uri.method = HTTP_GET;
	fonts_get_uri.handler = fonts_get_handler;

	favicon_get_uri.uri = "/favicon.ico";
	favicon_get_uri.method = HTTP_GET;
	favicon_get_uri.handler = favicon_get_handler;

	config_post_uri.uri = "/config";
	config_post_uri.method = HTTP_POST;
	config_post_uri.handler = config_post_handler;

	config_get_uri.uri = "/config";
	config_get_uri.method = HTTP_GET;
	config_get_uri.handler = config_get_handler;

	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.max_uri_handlers = 14;
	ESP_ERROR_CHECK(httpd_start(&server, &config));

	ESP_ERROR_CHECK(httpd_register_uri_handler(server, &index_get_uri));
	ESP_ERROR_CHECK(httpd_register_uri_handler(server, &script_get_uri));
	ESP_ERROR_CHECK(httpd_register_uri_handler(server, &gauge_get_uri));
	ESP_ERROR_CHECK(httpd_register_uri_handler(server, &moment_get_uri));
	ESP_ERROR_CHECK(httpd_register_uri_handler(server, &range_get_uri));
	ESP_ERROR_CHECK(httpd_register_uri_handler(server, &style_get_uri));
	ESP_ERROR_CHECK(httpd_register_uri_handler(server, &custom_get_uri));
	ESP_ERROR_CHECK(httpd_register_uri_handler(server, &fonts_get_uri));
	ESP_ERROR_CHECK(httpd_register_uri_handler(server, &favicon_get_uri));
	ESP_ERROR_CHECK(httpd_register_uri_handler(server, &config_post_uri));
	ESP_ERROR_CHECK(httpd_register_uri_handler(server, &config_get_uri));
	/* Make sure the number of slots are not exceeding, check config.max_uri_handlers*/
}

// 4.1 Function to handle POST requests for updating WiFi credentials
static esp_err_t config_post_handler(httpd_req_t *req)
{
	// Extract new SSID and password from the POST request content
	char new_ssid[32];
	char new_password[64];
	int led_state = 0;
	char buf[100];
	bool found = false;

	int ret = 0;
	int remaining = req->content_len;
	ESP_LOGI(TAG, "Context length: %d", req->content_len);

	while (remaining > 0)
	{
		memset(buf, 0, sizeof(buf));
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
		if (findStringValueForKey(buf, "ssid", new_ssid, sizeof(new_ssid)) != true)
		{
			ESP_LOGE(TAG, "ssid not sent");
			found = false;
		}
		if (findStringValueForKey(buf, "password", new_password, sizeof(new_password)) != true)
		{
			ESP_LOGE(TAG, "password not sent");
			found = false;
		}

		if (findIntValueForKey(buf, "led", &led_state) == true)
		{
			ESP_LOGI(TAG, "Led state = %d", led_state);
			if (led_state == 1)
			{
				// ON
				peripheral_gpio_on();
			}
			else
			{
				// OFF
				peripheral_gpio_off();
			}
		}

		if (found)
		{
			/* Replace '+' with space */
			utils_replace_characters(new_ssid, '+', ' ');

			/* Print on Debug port*/
			ESP_LOGI(TAG, "SSID: %s", new_ssid);
			ESP_LOGI(TAG, "PASS: %s", new_password);

			// 4.3 - save new wifi configuration into memory
			task_wifi_update_credentials(new_ssid, strlen(new_ssid), new_password, strlen(new_password));
		}
	}

	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, (const char *)buf, ret);

	// httpd_resp_send(req, NULL, 0);
	return ESP_OK;
}

static esp_err_t config_get_handler(httpd_req_t *req)
{
	char buf_rx[200];
	char buf_tx[200];
	uint16_t buf_rx_len = 0;
	uint16_t buf_tx_len = 0;

	/* Read URL query string length and allocate memory for length + 1,
	 * extra byte for null termination */
	buf_rx_len = httpd_req_get_url_query_len(req) + 1;
	if (buf_rx_len > 1 && buf_rx_len < sizeof(buf_rx))
	{
		// /* Clean the buffers */
		memset(buf_tx, 0, sizeof(buf_tx));
		memset(buf_rx, 0, sizeof(buf_rx));
		if (httpd_req_get_url_query_str(req, buf_rx, buf_rx_len) == ESP_OK)
		{
			ESP_LOGI(TAG, "Found URL query => %s", buf_rx);
			char param[32];

			buf_tx_len += snprintf(buf_tx + buf_tx_len, sizeof(buf_tx) - buf_tx_len, "{\"result\": \"success\", \"report\": {");

			/* Get value of expected key from query string */
			if (httpd_query_key_value(buf_rx, "ssid", param, sizeof(param)) == ESP_OK)
			{
				memset(param, 0, sizeof(param));
				task_wifi_get_credentials(param, sizeof(param), NULL, 0);
				if ('\0' != param[0])
				{
					buf_tx_len += snprintf(buf_tx + buf_tx_len, sizeof(buf_tx) - buf_tx_len, "\"ssid\": \"%s\"", param);
				}
				else
				{
					buf_tx_len += snprintf(buf_tx + buf_tx_len, sizeof(buf_tx) - buf_tx_len, "\"ssid\": \"not found\"");
				}
			}
			if (httpd_query_key_value(buf_rx, "rssi", param, sizeof(param)) == ESP_OK)
			{
				buf_tx_len += snprintf(buf_tx + buf_tx_len, sizeof(buf_tx) - buf_tx_len, ",\"rssi\": \"%s\"", get_wifi_status());
			}
			if (httpd_query_key_value(buf_rx, "status", param, sizeof(param)) == ESP_OK)
			{
				buf_tx_len += snprintf(buf_tx + buf_tx_len, sizeof(buf_tx) - buf_tx_len, ",\"status\": \"%s\"", is_wifi_connected() ? "Connected" : "Disconnected");
			}
			if (httpd_query_key_value(buf_rx, "adc_34", param, sizeof(param)) == ESP_OK)
			{
				buf_tx_len += snprintf(buf_tx + buf_tx_len, sizeof(buf_tx) - buf_tx_len, ",\"adc_34\": \"%d\"", peripheral_adc_value_io_34());
			}
			if (httpd_query_key_value(buf_rx, "adc_35", param, sizeof(param)) == ESP_OK)
			{
				buf_tx_len += snprintf(buf_tx + buf_tx_len, sizeof(buf_tx) - buf_tx_len, ",\"adc_35\": \"%d\"", peripheral_adc_value_io_35());
			}

			buf_tx_len += snprintf(buf_tx + buf_tx_len, sizeof(buf_tx) - buf_tx_len, "}}\r\n");
		}
	}
	else
	{
		ESP_LOGE(TAG, "Buffer length = %d (received = %d)", sizeof(buf_rx), buf_rx_len);
		httpd_resp_set_status(req, "300 length issue");
		httpd_resp_send(req, NULL, 0);

		return ESP_FAIL;
	}

	httpd_resp_set_status(req, "200 success");
	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, (const char *)buf_tx, buf_tx_len);

	return ESP_OK;
}

// 4.6 - Function to handle GET requests for the configuration page
static esp_err_t index_get_handler(httpd_req_t *req)
{
	if (NULL == req)
	{
		ESP_LOGE(TAG, "Param error");
		return ESP_FAIL;
	}

	extern const unsigned char file_start[] asm("_binary_index_html_start");
	extern const unsigned char file_end[] asm("_binary_index_html_end");
	const size_t file_size = (file_end - file_start);
	httpd_resp_set_type(req, "text/html");
	httpd_resp_send(req, (const char *)file_start, file_size);
	return ESP_OK;
}

static esp_err_t script_get_handler(httpd_req_t *req)
{
	if (NULL == req)
	{
		ESP_LOGE(TAG, "Param error");
		return ESP_FAIL;
	}

	extern const unsigned char script_start[] asm("_binary_scripts_js_start");
	extern const unsigned char script_end[] asm("_binary_scripts_js_end");
	const size_t script_size = (script_end - script_start);
	httpd_resp_set_type(req, "text/javascript");
	httpd_resp_send(req, (const char *)script_start, script_size);
	return ESP_OK;
}

static esp_err_t gauge_get_handler(httpd_req_t *req)
{
	if (NULL == req)
	{
		ESP_LOGE(TAG, "Param error");
		return ESP_FAIL;
	}

	extern const unsigned char gauge_script_start[] asm("_binary_gauge_min_js_start");
	extern const unsigned char gauge_script_end[] asm("_binary_gauge_min_js_end");
	const size_t gauge_script_size = (gauge_script_end - gauge_script_start);
	httpd_resp_set_type(req, "text/javascript");
	httpd_resp_send(req, (const char *)gauge_script_start, gauge_script_size);
	return ESP_OK;
}

static esp_err_t moment_get_handler(httpd_req_t *req)
{
	if (NULL == req)
	{
		ESP_LOGE(TAG, "Param error");
		return ESP_FAIL;
	}

	extern const unsigned char moment_script_start[] asm("_binary_moment_min_js_start");
	extern const unsigned char moment_script_end[] asm("_binary_moment_min_js_end");
	const size_t moment_script_size = (moment_script_end - moment_script_start);
	httpd_resp_set_type(req, "text/javascript");
	httpd_resp_send(req, (const char *)moment_script_start, moment_script_size);
	return ESP_OK;
}

static esp_err_t range_get_handler(httpd_req_t *req)
{
	if (NULL == req)
	{
		ESP_LOGE(TAG, "Param error");
		return ESP_FAIL;
	}

	extern const unsigned char range_script_start[] asm("_binary_range_min_js_start");
	extern const unsigned char range_script_end[] asm("_binary_range_min_js_end");
	const size_t range_script_size = (range_script_end - range_script_start);
	httpd_resp_set_type(req, "text/javascript");
	httpd_resp_send(req, (const char *)range_script_start, range_script_size);
	return ESP_OK;
}

static esp_err_t style_get_handler(httpd_req_t *req)
{
	if (NULL == req)
	{
		ESP_LOGE(TAG, "Param error");
		return ESP_FAIL;
	}

	extern const unsigned char style_start[] asm("_binary_bootstrap_min_css_start");
	extern const unsigned char style_end[] asm("_binary_bootstrap_min_css_end");
	const size_t style_size = (style_end - style_start);
	httpd_resp_set_type(req, "text/css");
	httpd_resp_send(req, (const char *)style_start, style_size);
	return ESP_OK;
}

static esp_err_t custom_get_handler(httpd_req_t *req)
{
	if (NULL == req)
	{
		ESP_LOGE(TAG, "Param error");
		return ESP_FAIL;
	}

	extern const unsigned char custom_start[] asm("_binary_custom_min_css_start");
	extern const unsigned char custom_end[] asm("_binary_custom_min_css_end");
	const size_t custom_size = (custom_end - custom_start);
	httpd_resp_set_type(req, "text/css");
	httpd_resp_send(req, (const char *)custom_start, custom_size);
	return ESP_OK;
}

static esp_err_t fonts_get_handler(httpd_req_t *req)
{
	if (NULL == req)
	{
		ESP_LOGE(TAG, "Param error");
		return ESP_FAIL;
	}

	extern const unsigned char fonts_css_start[] asm("_binary_fonts_css_start");
	extern const unsigned char fonts_css_end[] asm("_binary_fonts_css_end");
	const size_t fonts_css_size = (fonts_css_end - fonts_css_start);
	httpd_resp_set_type(req, "text/css");
	httpd_resp_send(req, (const char *)fonts_css_start, fonts_css_size);
	return ESP_OK;
}

static esp_err_t favicon_get_handler(httpd_req_t *req)
{
	if (NULL == req)
	{
		ESP_LOGE(TAG, "Param error");
		return ESP_FAIL;
	}

	extern const unsigned char favicon_ico_start[] asm("_binary_favicon_ico_start");
	extern const unsigned char favicon_ico_end[] asm("_binary_favicon_ico_end");
	const size_t favicon_ico_size = (favicon_ico_end - favicon_ico_start);
	httpd_resp_set_type(req, "image/x-icon");
	httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_size);
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
			return "Excellent";
		}
		else if (ap_info.rssi >= -60)
		{
			return "Good";
		}
		else if (ap_info.rssi >= -70)
		{
			return "Fair";
		}
		else
		{
			return "Poor";
		}
	}
	else
	{
		return "Not Connected";
	}
}
