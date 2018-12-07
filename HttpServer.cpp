#include <vector>
#include <Arduino.h>
#include "Config.h"
#include "HttpServer.h"

#include "AutoShutdown.h"
#include <EEPROMInterface.h>
#include <httpd.h>
#include <mico.h>
#include <EMW10xxInterface.h>
#include <SystemVariables.h>
#include <parson.h>
#include <OTAFirmwareUpdate.h>
#include <AZ3166WiFi.h>
#include <ReButton.h>
#include "Display.h"

#define POWER_OFF_TIME	(1000)	// [msec.]

#define app_httpd_log(...)

#define HTTPD_HDR_DEFORT (HTTPD_HDR_ADD_SERVER|HTTPD_HDR_ADD_CONN_CLOSE|HTTPD_HDR_ADD_PRAGMA_NO_CACHE)

extern NetworkInterface *network;

static const char* HTML_HEADER = \
	"<!DOCTYPE html>" \
	"<html lang=\"en\">" \
		"<head> " \
			"<meta charset=\"UTF-8\"> " \
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> " \
			"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\"> " \
			"<title>ReButton</title> " \
			"<style>" \
				"@charset \"UTF-8\";" \
					"html{font-size: 16px;}" \
					"html, *{font-family: -apple-system, BlinkMacSystemFont,\"Segoe UI\",\"Roboto\", \"Droid Sans\",\"Helvetica Neue\", Helvetica, Arial, sans-serif; line-height: 1.5; -webkit-text-size-adjust: 100%;}" \
					"*{font-size: 1rem;}" \
					"body{margin: 0; color: #212121; background: #f8f8f8;}" \
					"section{display: block;}" \
					"input{overflow: visible;}" \
					"h1, h2, h3{line-height: 1.2rem; margin: 0.75rem 0.5rem; font-weight: 500;}" \
					"h2 small{color: #424242; display: block; margin 0; white-space: nowrap}" \
					"h3 small{color: #424242; display: block; margin-top: -0.25rem; white-space: nowrap}" \
					"h1{font-size: 2rem;}" \
					"h2{font-size: 1.6875rem;}" \
					"h3{font-size: 1rem;}" \
					"p{margin: 0.5rem;}" \
					"small{font-size: 0.75em;}" \
					"a{color: #0277bd; text-decoration: underline; opacity: 1; transition: opacity 0.3s;}" \
					"a:visited{color: #01579b;}" \
					"a:hover, a:focus{opacity: 0.75;}" \
				"/*Definitions for the grid system.*/" \
					".container{margin: 0 10rem; padding: 0 0.75rem;}" \
					".row{box-sizing: border-box; display: -webkit-box; -webkit-box-flex: 0; -webkit-box-orient: horizontal; -webkit-box-direction: normal; display: -webkit-flex; display: flex; -webkit-flex: 0 1 auto; flex: 0 1 auto; -webkit-flex-flow: row wrap; flex-flow: row wrap;}" \
					"[class^='col-sm-']{box-sizing: border-box; -webkit-box-flex: 0; -webkit-flex: 0 0 auto; flex: 0 0 auto; padding: 0 0.25rem;}" \
					".col-sm-10{max-width: 90%; -webkit-flex-basis: 90%; flex-basis: 90%;}" \
					".col-sm-offset-0{margin: 0; padding: 0;}" \
					".col-sm-offset-1{margin: 0.25rem; padding: 0.5rem 0.5rem;}" \
				"/*Definitions for navigation elements.*/" \
					"header{display: block; height: 2.75rem; background: #8BC31F; color: #f5f5f5; padding: 0.125rem 0.5rem; white-space: nowrap; overflow-x: auto; overflow-y: hidden;}" \
					"header .logo{color: #f5f5f5; font-size: 1.35rem; line-height: 1.8125em; margin: 0.0625rem 0.375rem 0.0625rem 0.0625rem; transition: opacity 0.3s;}" \
					"header .logo{text-decoration: none;}" \
				"/*Definitions for forms and input elements.*/" \
					"form{background: #eeeeee; border: 1px solid #c9c9c9; margin: 0.5rem; padding: 0;}" \
					".input-group{display: inline-block;}" \
					"[type=\"text\"], [type=\"password\"], [type=\"url\"], [type=\"number\"], select{box-sizing: border-box; background: #fafafa; color: #212121; border: 1px solid #c9c9c9; border-radius: 2px; margin: 0.25rem; padding: 0.5rem 0.5rem;}" \
					"input:not([type=\"button\"]):not([type=\"submit\"]):not([type=\"reset\"]):hover, input:not([type=\"button\"]):not([type=\"submit\"]):not([type=\"reset\"]):focus, select:hover, select:focus{border-color: #0288d1; box-shadow: none;}" \
					"input:not([type=\"button\"]):not([type=\"submit\"]):not([type=\"reset\"]):disabled, select:disabled{cursor: not-allowed; opacity: 0.75;}" \
					"::-webkit-input-placeholder{opacity: 1; color: #616161;}" \
					"::-moz-placeholder{opacity: 1; color: #616161;}" \
					"::-ms-placeholder{opacity: 1; color: #616161;}" \
					"::placeholder{opacity: 1; color: #616161;}" \
					"button::-moz-focus-inner, [type=\"submit\"]::-moz-focus-inner{border-style: none; padding: 0;}" \
					"button, [type=\"submit\"]{-webkit-appearance: button;}" \
					"button{overflow: visible; text-transform: none;}" \
					"button, [type=\"submit\"], a.button, .button{display: inline-block; background: rgba(208, 208, 208, 0.75); color: #212121; border: 0; border-radius: 2px; padding: 0.5rem 0.75rem; margin: 0.5rem; text-decoration: none; transition: background 0.3s; cursor: pointer;}" \
					"button:hover, button:focus, [type=\"submit\"]:hover, [type=\"submit\"]:focus, a.button:hover, a.button:focus, .button:hover, .button:focus{background: #d0d0d0; opacity: 1;}" \
					"button:disabled, [type=\"submit\"]:disabled, a.button:disabled, .button:disabled{cursor: not-allowed; opacity: 0.75;}" \
				"/*Custom elements for forms and input elements.*/" \
					"button.primary, [type=\"submit\"].primary, .button.primary{background: rgba(143, 195, 31, 1); color: #fafafa;}" \
					"button.primary:hover, button.primary:focus, [type=\"submit\"].primary:hover, [type=\"submit\"].primary:focus, .button.primary:hover, .button.primary:focus{background: #004966;}" \
					"#content{margin-top: 2em;}" \
			"</style>" \
		"</head>";

static const char* HTML_INDEX = \
		"%s<body> " \
			"<header> " \
				"<h1 class=\"logo\">ReButton - Home</h1> " \
			"</header> " \
			"<section class=\"container\"> " \
				"<div id=\"content\" class=\"row\"> " \
					"<div class=\"col-sm-10 col-sm-offset-1\" style=\"text-align:center;\"> " \
						"<p><a href=\"/wifi\" target=\"_self\">Wi-Fi</a></p>" \
						"<p><a href=\"/iotcentral\" target=\"_self\">Azure IoT Central</a></p>" \
						"<p><a href=\"/iothub\" target=\"_self\">Azure IoT Hub</a></p>" \
						"<p><a href=\"/message\" target=\"_self\">Device to Cloud (D2C) Message</a></p>" \
						"<p><a href=\"/firmware\" target=\"_self\">Firmware Update</a></p>" \
					"</div>" \
					"<div class=\"col-sm-10 col-sm-offset-1\" style=\"text-align:center;\"> " \
						"<button type=\"button\" class=\"primary\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
					"</div>" \
				"</div>" \
			"</section>" \
		"</body>" \
	"</html>";

static const char* HTML_WIFI_A = \
		"<body> " \
			"<header> " \
				"<h1 class=\"logo\">ReButton - Wi-Fi</h1> " \
			"</header> " \
			"<section class=\"container\"> " \
				"<form action=\"wifi2\" method=\"post\" enctype=\"multipart/form-data\"> " \
					"<div id=\"content\" class=\"row\"> " \
						"<div class=\"input-group col-sm-10 col-sm-offset-1\"> " \
							"<label for=\"WiFiSSID\">Wi-Fi SSID : Select WiFi SSID to connect</label>" \
							"<select name=\"WiFiSSID\" id=\"WiFiSSID\" style=\"width:100%;\"> ";
static const char* HTML_WIFI_B = \
							"</select> " \
						"</div>" \
						"<div class=\"input-group col-sm-10 col-sm-offset-1\"> " \
   						"<h3>Please refresh browser to re-scan SSID if your SSID is not in the list above</h3>" \
						"</div>" \
						"<div class=\"input-group col-sm-10 col-sm-offset-1\"> " \
							"<label for=\"WiFiPassword\">Wi-Fi Passphase : Specify passphrase for SSID</label>" \
							"<input name=\"WiFiPassword\" id=\"WiFiPassword\" value=\"\" type=\"password\" style=\"width:100%%;\"> " \
						"</div>" \
						"<div class=\"input-group col-sm-10 col-sm-offset-1\"> " \
							"<label for=\"TimeServer\">Time Server : Specify Internet Time Server (Optional)</label>" \
							"<input name=\"TimeServer\" id=\"TimeServer\" value=\"%s\" type=\"text\" style=\"width:100%%;\"> " \
						"</div>" \
						"<div class=\"col-sm-10 col-sm-offset-1\"> " \
							"<button type=\"submit\" class=\"primary\">Save</button> " \
							"<button type=\"button\" class=\"primary\" onclick=\"location.href='/'\">Home</button>" \
							"<button type=\"button\" class=\"primary\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
						"</div>" \
					"</div>" \
				"</form> " \
			"</section>" \
		"</body>" \
	"</html>";

static const char* HTML_WIFI2_SUCCESS = \
		"%s<body> " \
			"<header> " \
				"<h1 class=\"logo\">ReButton - Wi-Fi</h1> " \
			"</header> " \
			"<section class=\"container\"> " \
				"<div id=\"content\" class=\"row\"> " \
					"<div class=\"col-sm-10 col-sm-offset-1\" style=\"text-align:center;\"> " \
						"<h2 style=\"color:#2e7d32;\">Wi-Fi saved.</h2> " \
					"</div>" \
					"<div class=\"col-sm-10 col-sm-offset-1\" style=\"text-align:center;\"> " \
						"<button type=\"button\" class=\"primary\" onclick=\"location.href='/'\">Home</button>" \
						"<button type=\"button\" class=\"primary\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
					"</div>" \
				"</div>" \
			"</section>" \
		"</body>" \
	"</html>";

static const char* HTML_IOTCENTRAL = \
		"%s<body> " \
			"<header> " \
				"<h1 class=\"logo\">ReButton - Azure IoT Central</h1> " \
			"</header> " \
			"<section class=\"container\"> " \
				"<form action=\"iotcentral2\" method=\"post\" enctype=\"multipart/form-data\"> " \
					"<div id=\"content\" class=\"row\"> " \
						"<div class=\"input-group col-sm-10 col-sm-offset-0\"> " \
							"<h2>Connect ReButton to Azure IoT Central</h2> " \
							"<h3>More about Azure IoT Central : https://aka.ms/AzureIoTCentral</h3>" \
							"<h3>Please erase IoT Hub Connection String to connect to Azure IoT Central</h3>" \
						"</div>" \
						"<div class=\"input-group col-sm-10 col-sm-offset-1\"> " \
							"<label for=\"ScopeId\">Scope ID</label>" \
							"<input name=\"ScopeId\" id=\"ScopeId\" value=\"%s\" type=\"text\" style=\"width:100%%;\"> " \
						"</div>" \
						"<div class=\"input-group col-sm-10 col-sm-offset-1\"> " \
							"<label for=\"DeviceId\">Device ID</label>" \
							"<input name=\"DeviceId\" id=\"DeviceId\" value=\"%s\" type=\"text\" style=\"width:100%%;\"> " \
						"</div>" \
						"<div class=\"input-group col-sm-10 col-sm-offset-1\"> " \
							"<label for=\"SasKey\">SAS Key : Please enter either Primary or Secondary key</label>" \
							"<input name=\"SasKey\" id=\"SasKey\" value=\"%s\" type=\"text\" style=\"width:100%%;\"> " \
						"</div>" \
						"<div class=\"col-sm-10 col-sm-offset-1\"> " \
							"<button type=\"submit\" class=\"primary\">Save</button> " \
							"<button type=\"button\" class=\"primary\" onclick=\"location.href='/'\">Home</button>" \
							"<button type=\"button\" class=\"primary\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
						"</div>" \
					"</div>" \
				"</form> " \
			"</section>" \
		"</body>" \
	"</html>";

static const char* HTML_IOTCENTRAL2_SUCCESS = \
		"%s<body> " \
			"<header> " \
				"<h1 class=\"logo\">ReButton - Azure IoT Central</h1> " \
			"</header> " \
			"<section class=\"container\"> " \
				"<div id=\"content\" class=\"row\" > " \
					"<div class=\"col-sm-10 col-sm-offset-1\" style=\"text-align:center;\"> " \
						"<h2 style=\"color:#2e7d32;\">Azure IoT Central saved.</h2> " \
					"</div>" \
					"<div class=\"col-sm-10 col-sm-offset-1\" style=\"text-align:center;\"> " \
						"<button type=\"button\" class=\"primary\" onclick=\"location.href='/'\">Home</button>" \
						"<button type=\"button\" class=\"primary\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
					"</div>" \
				"</div>" \
			"</section>" \
		"</body>" \
	"</html>";

static const char* HTML_IOTHUB = \
		"%s<body> " \
			"<header> " \
				"<h1 class=\"logo\">ReButton - Azure IoT Hub</h1> " \
			"</header> " \
			"<section class=\"container\"> " \
				"<form action=\"iothub2\" method=\"post\" enctype=\"multipart/form-data\"> " \
					"<div id=\"content\" class=\"row\"> " \
						"<div class=\"input-group col-sm-10 col-sm-offset-0\"> " \
							"<h2>Connect ReButton to Azure IoT Hub</h2> " \
							"<h3>More about Azure IoT Hub : https://aka.ms/iothub </h3>" \
						"</div>" \
						"<div class=\"input-group col-sm-10 col-sm-offset-1\"> " \
							"<label for=\"IoTHubConnectionString\">Azure IoT Hub connection string</label>" \
							"<input name=\"IoTHubConnectionString\" id=\"IoTHubConnectionString\" value=\"%s\" type=\"text\" style=\"width:100%%;\"> " \
						"</div>" \
						"<div class=\"col-sm-10 col-sm-offset-1\"> " \
							"<button type=\"submit\" class=\"primary\">Save</button> " \
							"<button type=\"button\" class=\"primary\" onclick=\"location.href='/'\">Home</button>" \
							"<button type=\"button\" class=\"primary\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
						"</div>" \
					"</div>" \
				"</form> " \
			"</section>" \
		"</body>" \
	"</html>";

static const char* HTML_IOTHUB2_SUCCESS = \
		"%s<body> " \
			"<header> " \
				"<h1 class=\"logo\">ReButton - IoT Hub</h1> " \
			"</header> " \
			"<section class=\"container\"> " \
				"<div id=\"content\" class=\"row\"> " \
					"<div class=\"col-sm-10 col-sm-offset-1\" style=\"text-align:center;\"> " \
						"<h2 style=\"color:#2e7d32;\">Azure IoT Hub saved.</h2> " \
					"</div>" \
					"<div class=\"col-sm-10 col-sm-offset-1\" style=\"text-align:center;\"> " \
						"<button type=\"button\" class=\"primary\" onclick=\"location.href='/'\">Home</button>" \
						"<button type=\"button\" class=\"primary\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
					"</div>" \
				"</div>" \
			"</section>" \
		"</body>" \
	"</html>";

static const char* HTML_MESSAGE = \
		"%s<body> " \
			"<header> " \
				"<h1 class=\"logo\">ReButton - Message</h1> " \
			"</header> " \
			"<section class=\"container\"> " \																																															
				"<form action=\"message2\" method=\"post\" enctype=\"multipart/form-data\"> " \
					"<div id=\"content\" class=\"row\"> " \
						"<div class=\"input-group col-sm-10 col-sm-offset-0\"> " \
							"<h2>Message Settings</h2> " \
							"<h3>Customize messages for button press events </h3>" \
						"</div>" \
						"<div class=\"input-group col-sm-10 col-sm-offset-1\"> " \
							"<label for=\"Message1\">Single click message</label>" \
							"<input name=\"Message1\" id=\"Message1\" value=\"%s\" type=\"text\" style=\"width:100%%;\"> " \
						"</div>" \
						"<div class=\"input-group col-sm-10 col-sm-offset-1\"> " \
							"<label for=\"Message2\">Double click message</label>" \
							"<input name=\"Message2\" id=\"Message2\" value=\"%s\" type=\"text\" style=\"width:100%%;\"> " \
						"</div>" \
						"<div class=\"input-group col-sm-10 col-sm-offset-1\"> " \
							"<label for=\"Message3\">Triple click message</label>" \
							"<input name=\"Message3\" id=\"Message3\" value=\"%s\" type=\"text\" style=\"width:100%%;\"> " \
						"</div>" \
						"<div class=\"input-group col-sm-10 col-sm-offset-1\"> " \
							"<label for=\"Message10\">Long press message (3 seconds)</label>" \
							"<input name=\"Message10\" id=\"Message10\" value=\"%s\" type=\"text\" style=\"width:100%%;\"> " \
						"</div>" \
						"<div class=\"input-group col-sm-10 col-sm-offset-1\"> " \
							"<label for=\"Message11\">Super long press message (6 Seconds)</label>" \
							"<input name=\"Message11\" id=\"Message11\" value=\"%s\" type=\"text\" style=\"width:100%%;\"> " \
						"</div>" \
						"<div class=\"col-sm-10 col-sm-offset-1\"> " \
							"<button type=\"submit\" class=\"primary\">Save</button> " \
							"<button type=\"button\" class=\"primary\" onclick=\"location.href='/'\">Home</button>" \
							"<button type=\"button\" class=\"primary\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
						"</div>" \
					"</div>" \
				"</form> " \
			"</section>" \
		"</body>" \
	"</html>";

static const char* HTML_MESSAGE2_SUCCESS = \
		"%s<body> " \
			"<header> " \
				"<h1 class=\"logo\">ReButton - Message</h1> " \
			"</header> " \
			"<section class=\"container\"> " \
				"<div id=\"content\" class=\"row\"> " \
					"<div class=\"col-sm-10 col-sm-offset-1\" style=\"text-align:center;\"> " \
						"<h2 style=\"color:#2e7d32;\">Message saved.</h2> " \
					"</div>" \
					"<div class=\"col-sm-10 col-sm-offset-1\" style=\"text-align:center;\"> " \
						"<button type=\"button\" class=\"primary\" onclick=\"location.href='/'\">Home</button>" \
						"<button type=\"button\" class=\"primary\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
					"</div>" \
				"</div>" \
			"</section>" \
		"</body>" \
	"</html>";

static const char* HTML_FIRMWARE_UPDATE = \
		"%s<body> " \
			"<header> " \
				"<h1 class=\"logo\">ReButton - Firmware Update</h1> " \
			"</header> " \
			"<section class=\"container\"> " \
				"<form action=\"firmware2\" method=\"post\" enctype=\"multipart/form-data\"> " \
					"<div id=\"content\" class=\"row\"> " \
						"<div class=\"input-group col-sm-10 col-sm-offset-0\"> " \
							"<h2>Current firmware version : %s</h2>" \
						"</div>" \
						"<div class=\"input-group col-sm-10 col-sm-offset-1\"> " \
							"<label for=\"PackageURI\">Package URI</label>" \
							"<input name=\"PackageURI\" id=\"PackageURI\" value=\"\" type=\"url\" style=\"width:100%%;\"> " \
						"</div>" \
						"<div class=\"input-group col-sm-10 col-sm-offset-1\"> " \
							"<label for=\"PackageCRC\">Package CRC</label>" \
							"<input name=\"PackageCRC\" id=\"PackageCRC\" value=\"\" type=\"text\" style=\"width:100%%;\"> " \
						"</div>" \
						"<div class=\"input-group col-sm-10 col-sm-offset-1\"> " \
							"<label for=\"PackageSize\">Package size</label>" \
							"<input name=\"PackageSize\" id=\"PackageSize\" value=\"\" type=\"number\" style=\"width:100%%;\"> " \
						"</div>" \
						"<div class=\"col-sm-10 col-sm-offset-1\"> " \
							"<button type=\"submit\" class=\"primary\">Update</button> " \
							"<button type=\"button\" class=\"primary\" onclick=\"location.href='/'\">Home</button>" \
							"<button type=\"button\" class=\"primary\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
						"</div>" \
					"</div>" \
				"</form> " \
			"</section>" \
		"</body>" \
	"</html>";

static const char* HTML_FIRMWARE_UPDATE2 = \
		"%s<body> " \
			"<header> " \
				"<h1 class=\"logo\">ReButton - Firmware Update</h1> " \
			"</header> " \
			"<section class=\"container\"> " \
				"<div id=\"content\" class=\"row\"> " \
					"<div class=\"col-sm-10 col-sm-offset-1\" style=\"text-align:center;\"> " \
						"<h2 style=\"color:#2e7d32;\">Firmware updating...</h2> " \
						"<h5>Wait a few seconds for the ReButton to shutdown...</h5> " \
					"</div>" \
				"</div>" \
			"</section>" \
		"</body>" \
	"</html>";

static const char* HTML_SHUTDOWN = \
		"%s<body> " \
			"<header> " \
				"<h1 class=\"logo\">ReButton - Shutdown</h1> " \
			"</header> " \
			"<section class=\"container\"> " \
				"<div id=\"content\" class=\"row\"> " \
					"<div class=\"col-sm-10 col-sm-offset-1\" style=\"text-align:center;\"> " \
						"<h2 style=\"color:#2e7d32;\">Shutdown.</h2> " \
					"</div>" \
				"</div>" \
			"</section>" \
		"</body>" \
	"</html>";

static bool is_http_init;
static bool is_handlers_registered;

static String stringformat(int maxLength, const char* format, ...)
{
	va_list args;
	char* buf = new char[maxLength + 1];

	va_start(args, format);
	vsnprintf(buf, maxLength + 1, format, args);
	va_end(args);

	String str(buf);

	delete[] buf;

	return str;
}

static int write_eeprom(char* string, int idxZone)
{
	Serial.printf("write_eeprom(\"%s\",%d)\n", string, idxZone);
	return 0;
}

static OSStatus HttpdSend(httpd_request_t* req, const char* content)
{
	OSStatus err;

	err = httpd_send_all_header(req, HTTP_RES_200, strlen(content), HTTP_CONTENT_HTML_STR);
	require_noerr_action(err, exit, app_httpd_log("ERROR: Unable to send http result headers."));

	err = httpd_send_body(req->sock, (const unsigned char*)content, strlen(content));
	require_noerr_action(err, exit, app_httpd_log("ERROR: Unable to send http result body."));

exit:
	return err;
}

////////////////////////////////////////////////////////////////////////////////////
// HTML handlers

static int HtmlIndexGetHandler(httpd_request_t* req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	String html = stringformat(strlen(HTML_INDEX) + strlen(HTML_HEADER), HTML_INDEX, HTML_HEADER);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlWiFiGetHandler(httpd_request_t *req)
{
	AutoShutdownUpdateStartTime();

	int err;

	// scan network
	WiFiAccessPoint wifiScanResult[100];
	int wifiCount = ((EMW10xxInterface*)network)->scan(wifiScanResult, 100);

	int validWifiIndex[100];
	int validWifiCount = 0;
	for (int i = 0; i < wifiCount; i++)
	{
		// too weak
		if (wifiScanResult[i].get_rssi() < -100) continue;

		char* ssid = (char*)wifiScanResult[i].get_ssid();
		int ssidLen = strlen(ssid);

		if (strcmp(ssid, Config.APmodeSSID) == 0) continue;
		if (ssidLen < 1 || CONFIG_WIFI_SSID_MAX_LEN < ssidLen) continue;

		bool shouldSkip = false;
		for (int j = 0; j < i; j++)
		{
			if (strcmp(ssid, wifiScanResult[j].get_ssid()) == 0)
			{
				// duplicate ap name
				shouldSkip = true;
				break;
			}
		}
		if (shouldSkip) continue;

		validWifiIndex[validWifiCount++] = i;
		if (validWifiCount >= sizeof(validWifiIndex) / sizeof(validWifiIndex[0])) break;
	}

	String html = HTML_HEADER;
	html += HTML_WIFI_A;
	for (int i = 0; i < validWifiCount; i++)
	{
		char* ssid = (char*)wifiScanResult[validWifiIndex[i]].get_ssid();
		int ssidLen = strlen(ssid);
		html += stringformat(15 + ssidLen + 2 + ssidLen + 9, "<option value=\"%s\">%s</option>", ssid, ssid);
	}
	html += stringformat(strlen(HTML_WIFI_B) + strlen(Config.TimeServer), HTML_WIFI_B, Config.TimeServer);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlWiFi2PostHandler(httpd_request_t *req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	std::vector<char> buf(1000);

	if ((err = httpd_get_data(req, &buf[0], buf.size())) != kNoErr) return err;
	if (strstr(req->content_type, "multipart/form-data") == NULL) return kGeneralErr;
	char* boundary = strstr(req->content_type, "boundary=");
	boundary += 9;

	char wifiSSID[CONFIG_WIFI_SSID_MAX_LEN + 1];
	char wifiPassword[CONFIG_WIFI_PASSWORD_MAX_LEN + 1];
	char timeServer[CONFIG_TIME_SERVER_MAX_LEN + 1];

	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "WiFiSSID", wifiSSID, CONFIG_WIFI_SSID_MAX_LEN)) != kNoErr) return err;
	if (strlen(wifiSSID) <= 0) return kGeneralErr;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "WiFiPassword", wifiPassword, CONFIG_WIFI_PASSWORD_MAX_LEN)) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "TimeServer", timeServer, sizeof(timeServer))) != kNoErr) return err;

	strncpy(Config.WiFiSSID, wifiSSID, sizeof(Config.WiFiSSID));
	strncpy(Config.WiFiPassword, wifiPassword, sizeof(Config.WiFiPassword));
	strncpy(Config.TimeServer, timeServer, sizeof(Config.TimeServer));
	ConfigWrite();

	String html = stringformat(strlen(HTML_WIFI2_SUCCESS) + strlen(HTML_HEADER), HTML_WIFI2_SUCCESS, HTML_HEADER);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlIoTCentralGetHandler(httpd_request_t* req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	String html = stringformat(strlen(HTML_IOTCENTRAL) + strlen(HTML_HEADER) + strlen(Config.ScopeId) + strlen(Config.DeviceId) + strlen(Config.SasKey), HTML_IOTCENTRAL, HTML_HEADER, Config.ScopeId, Config.DeviceId, Config.SasKey);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlIoTCentral2PostHandler(httpd_request_t *req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	std::vector<char> buf(1000);

	if ((err = httpd_get_data(req, &buf[0], buf.size())) != kNoErr) return err;
	if (strstr(req->content_type, "multipart/form-data") == NULL) return kGeneralErr;
	char* boundary = strstr(req->content_type, "boundary=");
	boundary += 9;

	char scopeId[CONFIG_SCOPE_ID_MAX_LEN + 1];
	char deviceId[CONFIG_DEVICE_ID_MAX_LEN + 1];
	char sasKey[CONFIG_SAS_KEY_MAX_LEN + 1];

	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "ScopeId", scopeId, CONFIG_SCOPE_ID_MAX_LEN)) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "DeviceId", deviceId, CONFIG_DEVICE_ID_MAX_LEN)) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "SasKey", sasKey, CONFIG_SAS_KEY_MAX_LEN)) != kNoErr) return err;

	strncpy(Config.ScopeId, scopeId, sizeof(Config.ScopeId));
	strncpy(Config.DeviceId, deviceId, sizeof(Config.DeviceId));
	strncpy(Config.SasKey, sasKey, sizeof(Config.SasKey));
	ConfigWrite();

	String html = stringformat(strlen(HTML_IOTCENTRAL2_SUCCESS) + strlen(HTML_HEADER), HTML_IOTCENTRAL2_SUCCESS, HTML_HEADER);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlIoTHubGetHandler(httpd_request_t* req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	String html = stringformat(strlen(HTML_IOTHUB) + strlen(HTML_HEADER) + strlen(Config.IoTHubConnectionString), HTML_IOTHUB, HTML_HEADER, Config.IoTHubConnectionString);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlIoTHub2PostHandler(httpd_request_t *req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	std::vector<char> buf(1000);

	if ((err = httpd_get_data(req, &buf[0], buf.size())) != kNoErr) return err;
	if (strstr(req->content_type, "multipart/form-data") == NULL) return kGeneralErr;
	char* boundary = strstr(req->content_type, "boundary=");
	boundary += 9;

	char connectionString[CONFIG_IOTHUB_CONNECTION_STRING_MAX_LEN + 1];

	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "IoTHubConnectionString", connectionString, CONFIG_IOTHUB_CONNECTION_STRING_MAX_LEN)) != kNoErr) return err;

	strncpy(Config.IoTHubConnectionString, connectionString, sizeof(Config.IoTHubConnectionString));
	ConfigWrite();

	String html = stringformat(strlen(HTML_IOTHUB2_SUCCESS) + strlen(HTML_HEADER), HTML_IOTHUB2_SUCCESS, HTML_HEADER);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlMessageGetHandler(httpd_request_t* req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	String html = stringformat(strlen(HTML_MESSAGE) + strlen(HTML_HEADER) + strlen(Config.Message1) + strlen(Config.Message2) + strlen(Config.Message3) + strlen(Config.Message10) + strlen(Config.Message11), HTML_MESSAGE, HTML_HEADER, Config.Message1, Config.Message2, Config.Message3, Config.Message10, Config.Message11);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlMessage2PostHandler(httpd_request_t *req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	std::vector<char> buf(1000);

	if ((err = httpd_get_data(req, &buf[0], buf.size())) != kNoErr) return err;
	if (strstr(req->content_type, "multipart/form-data") == NULL) return kGeneralErr;
	char* boundary = strstr(req->content_type, "boundary=");
	boundary += 9;

	char message1[CONFIG_MESSAGE_MAX_LEN + 1];
	char message2[CONFIG_MESSAGE_MAX_LEN + 1];
	char message3[CONFIG_MESSAGE_MAX_LEN + 1];
	char message10[CONFIG_MESSAGE_MAX_LEN + 1];
	char message11[CONFIG_MESSAGE_MAX_LEN + 1];

	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "Message1", message1, CONFIG_MESSAGE_MAX_LEN)) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "Message2", message2, CONFIG_MESSAGE_MAX_LEN)) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "Message3", message3, CONFIG_MESSAGE_MAX_LEN)) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "Message10", message10, CONFIG_MESSAGE_MAX_LEN)) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "Message11", message11, CONFIG_MESSAGE_MAX_LEN)) != kNoErr) return err;

	strncpy(Config.Message1, message1, sizeof(Config.Message1));
	strncpy(Config.Message2, message2, sizeof(Config.Message2));
	strncpy(Config.Message3, message3, sizeof(Config.Message3));
	strncpy(Config.Message10, message10, sizeof(Config.Message10));
	strncpy(Config.Message11, message11, sizeof(Config.Message11));
	ConfigWrite();

	String html = stringformat(strlen(HTML_MESSAGE2_SUCCESS) + strlen(HTML_HEADER), HTML_MESSAGE2_SUCCESS, HTML_HEADER);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlFirmwareUpdateGetHandler(httpd_request_t* req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	String html = stringformat(strlen(HTML_FIRMWARE_UPDATE) + strlen(HTML_HEADER) + strlen(CONFIG_FIRMWARE_VERSION), HTML_FIRMWARE_UPDATE, HTML_HEADER, CONFIG_FIRMWARE_VERSION);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlFirmwareUpdate2PostHandler(httpd_request_t* req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	if (!ReButton::IsButtonPressed())
	{
		String html = stringformat(strlen(HTML_FIRMWARE_UPDATE) + strlen(HTML_HEADER) + strlen(CONFIG_FIRMWARE_VERSION), HTML_FIRMWARE_UPDATE, HTML_HEADER, CONFIG_FIRMWARE_VERSION);
		if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

		return kNoErr;
	}

	std::vector<char> buf(1000);

	if ((err = httpd_get_data(req, &buf[0], buf.size())) != kNoErr) return err;
	if (strstr(req->content_type, "multipart/form-data") == NULL) return kGeneralErr;
	char* boundary = strstr(req->content_type, "boundary=");
	boundary += 9;

	char packageURI[100 + 1];
	char packageCRC[6 + 1];
	char packageSize[6 + 1];

	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "PackageURI", packageURI, sizeof (packageURI))) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "PackageCRC", packageCRC, sizeof(packageCRC))) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "PackageSize", packageSize, sizeof(packageSize))) != kNoErr) return err;

	Serial.printf("packageURI = %s\n", packageURI);
	Serial.printf("packageCRC = %s\n", packageCRC);
	Serial.printf("packageSize = %s\n", packageSize);

	char* endp;
	int inPackageCRC = strtol(packageCRC, &endp, 0);
	if (*endp != '\0') return kGeneralErr;
	int inPackageSize = strtol(packageSize, &endp, 0);
	if (*endp != '\0') return kGeneralErr;

	String html = stringformat(strlen(HTML_FIRMWARE_UPDATE2) + strlen(HTML_HEADER), HTML_FIRMWARE_UPDATE2, HTML_HEADER);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	DisplayColor(DISPLAY_OFF);
	do
	{
		Serial.println("Connect WiFi...");
		if (WiFi.begin(Config.WiFiSSID, Config.WiFiPassword) != WL_CONNECTED) break;
		Serial.println("Connected WiFi.");

		Serial.println("Download firmware...");
		uint16_t dlPackageCRC;
		int dlPackageSize = OTADownloadFirmware(packageURI, &dlPackageCRC);
		Serial.printf("Downloaded firmware.\n");
		Serial.printf("inPackageCRC = %#04x\n", inPackageCRC);
		Serial.printf("inPackageSize = %d\n", inPackageSize);
		Serial.printf("dlPackageCRC = %#04x\n", dlPackageCRC);
		Serial.printf("dlPackageSize = %d\n", dlPackageSize);

		if (dlPackageCRC != inPackageCRC || dlPackageSize != inPackageSize) break;

		Serial.println("Switch firmware...");
		if (OTAApplyNewFirmware(inPackageSize, inPackageCRC) != 0) break;
		Serial.println("Switched firmware.");

		delay(1000);
		mico_system_reboot();
	}
	while (false);

	for (;;)
	{
		for (int i = 0; i < 3; i++)
		{
			DisplayColor(DISPLAY_ERROR);
			delay(200);
			DisplayColor(DISPLAY_OFF);
			delay(200);
		}
		ReButton::PowerSupplyEnable(false);
		delay(POWER_OFF_TIME);
	}
}

static int HtmlShutdownGetHandler(httpd_request_t* req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	String html = stringformat(strlen(HTML_SHUTDOWN) + strlen(HTML_HEADER), HTML_SHUTDOWN, HTML_HEADER);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	delay(1000);
	ReButton::PowerSupplyEnable(false);

	return kNoErr;
}

////////////////////////////////////////////////////////////////////////////////////
// REST handlers

static int RestConfigGetHandler(httpd_request_t *req)
{
  Serial.println(__FUNCTION__);
  int err = kNoErr;
  return err;
}

static int RestConfigPostHandler(httpd_request_t *req)
{
  Serial.println(__FUNCTION__);
  OSStatus err = kNoErr;
  return err;  
}

/*
* REST API for IoT Hub
*/
static char *GetHostNameFromConnectionString(char *connectionString)
{
    if (connectionString == NULL)
    {
        return NULL;
    }
    int start = 0;
    int cur = 0;
    bool find = false;
    while (connectionString[cur] > 0)
    {
        if (connectionString[cur] == '=')
        {
            // Check the key
            if (memcmp(&connectionString[start], "HostName", 8) == 0)
            {
                // This is the host name
                find = true;
            }
            start = ++cur;
            // Value
            while (connectionString[cur] > 0)
            {
                if (connectionString[cur] == ';')
                {
                    break;
                }
                cur++;
            }
            if (find && cur - start > 0)
            {
                char *hostname = (char *)malloc(cur - start + 1);
                memcpy(hostname, &connectionString[start], cur - start);
                hostname[cur - start] = 0;
                return hostname;
            }
            start = cur + 1;
        }
        cur++;
    }
    return NULL;
}

static char *GetDeviceNameFromConnectionString(char *connectionString)
{
    if (connectionString == NULL)
    {
        return NULL;
    }
    int start = 0;
    int cur = 0;
    bool find = false;
    while (connectionString[cur] > 0)
    {
        if (connectionString[cur] == '=')
        {
            // Check the key
            if (memcmp(&connectionString[start], "DeviceId", 8) == 0)
            {
                // This is the device id
                find = true;
            }
            start = ++cur;
            // Value
            while (connectionString[cur] > 0)
            {
                if (connectionString[cur] == ';')
                {
                    break;
                }
                cur++;
            }
            if (find && cur - start > 0)
            {
                char *deviceName = (char *)malloc(cur - start + 1);
                memcpy(deviceName, &connectionString[start], cur - start);
                deviceName[cur - start] = 0;
                return deviceName;
            }
            start = cur + 1;
        }
        cur++;
    }
    return NULL;
}

static int RestIoTHubGetHandler(httpd_request_t *req)
{
  Serial.println(__FUNCTION__);
  EEPROMInterface eeprom;
  JSON_Value *root_value = json_value_init_object();
  JSON_Object *root_object = json_value_get_object(root_value);
  char *serialized_string = NULL;
  char *connString[AZ_IOT_HUB_MAX_LEN + 1] = { '\0' };
  int err = kNoErr;

  int ret = eeprom.read((uint8_t*)connString, AZ_IOT_HUB_MAX_LEN, 0x00, AZ_IOT_HUB_ZONE_IDX);

  if (ret < 0)
  {
      Serial.println("Unable to get the azure iot connection string from EEPROM. Please set the value in configuration mode.");
      return kGeneralErr;
  }

  char *iothub_hostname = GetHostNameFromConnectionString((char *)connString);
  char *iothub_deviceid = GetDeviceNameFromConnectionString((char *)connString);

  json_object_set_string(root_object, "iothub", iothub_hostname);
  json_object_set_string(root_object, "iotdevicename", iothub_deviceid);
  json_object_set_string(root_object, "iotdevicesecret", (char *)connString);
  serialized_string = json_serialize_to_string_pretty(root_value);
  puts(serialized_string);

  int json_length = strlen(serialized_string);
    
  err = httpd_send_all_header(req, HTTP_RES_200, json_length, HTTP_CONTENT_JSON_STR);
  require_noerr_action( err, exit, app_httpd_log("ERROR: Unable to send http headers.") );

  err = httpd_send_body(req->sock, (const unsigned char*)serialized_string, json_length);
  require_noerr_action( err, exit, app_httpd_log("ERROR: Unable to send http body.") );

exit:
  if (serialized_string)
  {
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
  }

  if (iothub_hostname)
  {
    free(iothub_hostname);
  }

  if (iothub_deviceid)
  {
    free(iothub_deviceid);
  }
  return err;
}

static int RestIoTHubPostHandler(httpd_request_t *req)
{
  Serial.println(__FUNCTION__);

  EEPROMInterface eeprom;
  OSStatus err = kNoErr;
  int buf_size = 1000;
  char *buf;

  buf = (char *)malloc(buf_size);
  err = httpd_get_data(req, buf, buf_size);
  app_httpd_log("httpd_get_data return value: %d", err);
  require_noerr( err, Save_Out );
  
  if (strstr(req->content_type, HTTP_CONTENT_JSON_STR) != NULL)
  {
    JSON_Value *root_value = NULL;
    root_value = json_parse_string(buf);
    JSON_Object *root_object;

    if (json_value_get_type(root_value) == JSONObject)
    {
        root_object = json_value_get_object(root_value);
        const char *strConnString = json_object_get_string(root_object, "connectionstring");

        err = write_eeprom((char *)strConnString, AZ_IOT_HUB_ZONE_IDX);

        if (err != 0)
        {
          return false;
        }

        Serial.println(strConnString);

        if(root_value)
        {
          json_value_free(root_value);
        }
    }
  }

Save_Out:

  err = httpd_send_all_header(req, HTTP_RES_200, 0, HTTP_CONTENT_JSON_STR);

exit:
  return err;  
}

/*
* REST API for WiFi
*/

static int RestWiFiGetHandler(httpd_request_t *req)
{
  Serial.println(__FUNCTION__);

  EEPROMInterface eeprom;
  char ssid[WIFI_SSID_MAX_LEN + 1] = { 0 };
  char pwd[WIFI_PWD_MAX_LEN + 1] = { 0 };

  int ret = eeprom.read((uint8_t*)ssid, WIFI_SSID_MAX_LEN, 0x00, WIFI_SSID_ZONE_IDX);

  if (ret < 0)
  {
      Serial.print("ERROR: Failed to get the Wi-Fi SSID from EEPROM.\r\n");
      return false;
  }

  ret = eeprom.read((uint8_t*)pwd, WIFI_PWD_MAX_LEN, 0x00, WIFI_PWD_ZONE_IDX);
  if (ret < 0)
  {
      Serial.print("ERROR: Failed to get the Wi-Fi password from EEPROM.\r\n");
      return false;
  }

  JSON_Value *root_value = json_value_init_object();
  JSON_Object *root_object = json_value_get_object(root_value);
  char *serialized_string = NULL;

  json_object_set_string(root_object, "ssid", ssid);
  json_object_set_string(root_object, "password", pwd);
  serialized_string = json_serialize_to_string_pretty(root_value);
  puts(serialized_string);

  int json_length = strlen(serialized_string);
  int err = kNoErr;
    
  err = httpd_send_all_header(req, HTTP_RES_200, json_length, HTTP_CONTENT_JSON_STR);
  require_noerr_action( err, exit, app_httpd_log("ERROR: Unable to send http headers.") );

  err = httpd_send_body(req->sock, (const unsigned char*)serialized_string, json_length);
  require_noerr_action( err, exit, app_httpd_log("ERROR: Unable to send http body.") );

exit:
  if (serialized_string)
  {
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
  }
  return err;
}

static int RestWiFiPostHandler(httpd_request_t *req)
{
  Serial.println(__FUNCTION__);
  OSStatus err = kNoErr;
  int buf_size = 1000;
  char *buf;

  buf = (char *)malloc(buf_size);
  err = httpd_get_data(req, buf, buf_size);
  app_httpd_log("httpd_get_data return value: %d", err);
  require_noerr( err, Save_Out );
  
  if (strstr(req->content_type, HTTP_CONTENT_JSON_STR) != NULL)
  {
    JSON_Value *root_value = NULL;
    root_value = json_parse_string(buf);
    JSON_Object *root_object;

    if (json_value_get_type(root_value) == JSONObject)
    {
      root_object = json_value_get_object(root_value);
      const char *strSSID = json_object_get_string(root_object, "ssid");
      const char *strPASS = json_object_get_string(root_object, "password");
      
      err = write_eeprom((char *)strSSID, WIFI_SSID_ZONE_IDX);
      if (err != 0)
      {
        return false;
      }
      err = write_eeprom((char *)strPASS, WIFI_PWD_ZONE_IDX);
      if (err != 0)
      {
        return false;
      }

      if(root_value)
      {
        json_value_free(root_value);
      }
    }
  }

Save_Out:

  err = httpd_send_all_header(req, HTTP_RES_200, 0, HTTP_CONTENT_JSON_STR);

exit:
  return err;  
}

/*
* REST API for timeserver
*/
static int RestTimeServerGetHandler(httpd_request_t *req)
{
  Serial.println(__FUNCTION__);
  JSON_Value *root_value = json_value_init_object();
  JSON_Object *root_object = json_value_get_object(root_value);
  char *serialized_string = NULL;
  //
  // ToDo: Read Time Server Setting from EEPROM
  //
  json_object_set_string(root_object, "timeserver", "MyTimeServer");
  serialized_string = json_serialize_to_string_pretty(root_value);
  puts(serialized_string);

  int json_length = strlen(serialized_string);
  int err = kNoErr;
    
  err = httpd_send_all_header(req, HTTP_RES_200, json_length, HTTP_CONTENT_JSON_STR);
  require_noerr_action( err, exit, app_httpd_log("ERROR: Unable to send http headers.") );

  err = httpd_send_body(req->sock, (const unsigned char*)serialized_string, json_length);
  require_noerr_action( err, exit, app_httpd_log("ERROR: Unable to send http body.") );

exit:
  if (serialized_string)
  {
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
  }
  return err;
}

static int RestTimeServerPostHandler(httpd_request_t *req)
{
  Serial.println(__FUNCTION__);
  OSStatus err = kNoErr;
  int buf_size = 1000;
  char *buf;

  buf = (char *)malloc(buf_size);
  err = httpd_get_data(req, buf, buf_size);
  app_httpd_log("httpd_get_data return value: %d", err);
  require_noerr( err, Save_Out );
  
  if (strstr(req->content_type, HTTP_CONTENT_JSON_STR) != NULL)
  {
    //
    // ToDo : Save time server to EEPROM
    //
    JSON_Value *root_value = NULL;
    root_value = json_parse_string(buf);
    JSON_Object *root_object;

    if (json_value_get_type(root_value) == JSONObject)
    {
      root_object = json_value_get_object(root_value);
      const char *strTimeServer = json_object_get_string(root_object, "timeserver");

      Serial.println(strTimeServer);

      if(root_value)
      {
        json_value_free(root_value);
      }
    }
  }

Save_Out:

  err = httpd_send_all_header(req, HTTP_RES_200, 0, HTTP_CONTENT_JSON_STR);

exit:
  return err;  
}

static int RestShutdownPost(httpd_request_t *req)
{
  Serial.println(__FUNCTION__);
  OSStatus err = kNoErr;
  int buf_size = 1000;
  char *buf;

  buf = (char *)malloc(buf_size);
  err = httpd_get_data(req, buf, buf_size);
  app_httpd_log("httpd_get_data return value: %d", err);
  require_noerr( err, Save_Out );
  
  if (strstr(req->content_type, HTTP_CONTENT_JSON_STR) != NULL)
  {
    JSON_Value *root_value = NULL;
    root_value = json_parse_string(buf);
    JSON_Object *root_object;

    if (json_value_get_type(root_value) == JSONObject)
    {
      root_object = json_value_get_object(root_value);
      double iDelay = json_object_get_number(root_object, "shutdowndelayinms");

      if(root_value)
      {
        json_value_free(root_value);
      }

      delay(iDelay);
      mico_system_reboot();
    }
  }

Save_Out:

  err = httpd_send_all_header(req, HTTP_RES_200, 0, HTTP_CONTENT_JSON_STR);

exit:
  return err;  
}

static struct httpd_wsgi_call g_app_handlers[] =
{
	{ "/"              , HTTPD_HDR_DEFORT, 0, HtmlIndexGetHandler         , NULL                          , NULL, NULL },
	{ "/wifi"          , HTTPD_HDR_DEFORT, 0, HtmlWiFiGetHandler          , NULL                          , NULL, NULL },
	{ "/wifi2"         , HTTPD_HDR_DEFORT, 0, NULL                        , HtmlWiFi2PostHandler          , NULL, NULL },
	{ "/iotcentral"    , HTTPD_HDR_DEFORT, 0, HtmlIoTCentralGetHandler    , NULL                          , NULL, NULL },
	{ "/iotcentral2"   , HTTPD_HDR_DEFORT, 0, NULL                        , HtmlIoTCentral2PostHandler    , NULL, NULL },
	{ "/iothub"        , HTTPD_HDR_DEFORT, 0, HtmlIoTHubGetHandler        , NULL                          , NULL, NULL },
	{ "/iothub2"       , HTTPD_HDR_DEFORT, 0, NULL                        , HtmlIoTHub2PostHandler        , NULL, NULL },
	{ "/message"       , HTTPD_HDR_DEFORT, 0, HtmlMessageGetHandler       , NULL                          , NULL, NULL },
	{ "/message2"      , HTTPD_HDR_DEFORT, 0, NULL                        , HtmlMessage2PostHandler       , NULL, NULL },
	{ "/firmware"      , HTTPD_HDR_DEFORT, 0, HtmlFirmwareUpdateGetHandler, NULL                          , NULL, NULL },
	{ "/firmware2"     , HTTPD_HDR_DEFORT, 0, NULL                        , HtmlFirmwareUpdate2PostHandler, NULL, NULL },
	{ "/shutdown"      , HTTPD_HDR_DEFORT, 0, HtmlShutdownGetHandler      , NULL                          , NULL, NULL },
	//{ "/api"           , HTTPD_HDR_DEFORT, 0, RestConfigGetHandler        , RestConfigPostHandler         , NULL, NULL },
	//{ "/api/iothub"    , HTTPD_HDR_DEFORT, 0, RestIoTHubGetHandler        , RestIoTHubPostHandler         , NULL, NULL },
	//{ "/api/wifi"      , HTTPD_HDR_DEFORT, 0, RestWiFiGetHandler          , RestWiFiPostHandler           , NULL, NULL },
	//{ "/api/timeserver", HTTPD_HDR_DEFORT, 0, RestTimeServerGetHandler    , RestTimeServerPostHandler     , NULL, NULL },
	//{ "/api/shutdown"  , HTTPD_HDR_DEFORT, 0, NULL                        , RestShutdownPost              , NULL, NULL },
};

static int g_app_handlers_no = sizeof(g_app_handlers) / sizeof(g_app_handlers[0]);

static void app_http_register_handlers()
{
  int rc;
  rc = httpd_register_wsgi_handlers(g_app_handlers, g_app_handlers_no);
  if (rc)
  {
    app_httpd_log("failed to register test web handler");
  }
}

static int _app_httpd_start()
{
  OSStatus err = kNoErr;
  app_httpd_log("initializing web-services");

  /*Initialize HTTPD*/
  if (is_http_init == false)
  {
    err = httpd_init();
    require_noerr_action( err, exit, app_httpd_log("failed to initialize httpd") );
    is_http_init = true;
  }

  /*Start http thread*/
  err = httpd_start();
  if (err != kNoErr)
  {
    app_httpd_log("failed to start httpd thread");
    httpd_shutdown();
  }
exit:
  return err;
}

int HttpServerStart()
{
  int err = kNoErr;
  err = _app_httpd_start();
  require_noerr( err, exit );

  if (is_handlers_registered == false)
  {
    app_http_register_handlers();
    is_handlers_registered = true;
  }

exit:
  return err;
}

int HttpServerStop()
{
  OSStatus err = kNoErr;

  /* HTTPD and services */
  app_httpd_log("stopping down httpd");
  err = httpd_stop();
  require_noerr_action( err, exit, app_httpd_log("failed to halt httpd") );

exit:
  return err;
}
