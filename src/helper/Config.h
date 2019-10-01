#pragma once

#include "TypeAndFunc.h"

#define CONFIG_AUTO_SHUTDOWN_TIMEOUT			(60000)		// [msec.]
#define CONFIG_AUTO_SHUTDOWN_TIMEOUT_AP			(600000)	// [msec.]

#define CONFIG_MESSAGE_MAX_LEN					(32)
#define CONFIG_PRODUCT_ID_MAX_LEN				(32)
#define CONFIG_PROPERTY_NAME_MAX_LEN			(32)
#define CONFIG_CUSTOM_MESSAGE_JSON_MAX_LEN		(200)
#define CONFIG_WIFI_SSID_MAX_LEN				(32)
#define CONFIG_WIFI_PASSWORD_MAX_LEN			(64)
#define CONFIG_TIME_SERVER_MAX_LEN				(32)
#define CONFIG_SCOPE_ID_MAX_LEN					(128)
#define CONFIG_DEVICE_ID_MAX_LEN				(128)
#define CONFIG_SAS_KEY_MAX_LEN					(128)
#define CONFIG_IOTHUB_CONNECTION_STRING_MAX_LEN	(512)
#define CONFIG_APMODE_SSID_MAX_LEN				(32)
#define CONFIG_APMODE_PASSWORD_MAX_LEN			(64)

struct CONFIG_TYPE
{
	DISPLAY_COLOR_TYPE DisplayColorSingleClick;
	DISPLAY_COLOR_TYPE DisplayColorDoubleClick;
	DISPLAY_COLOR_TYPE DisplayColorTripleClick;
	DISPLAY_COLOR_TYPE DisplayColorLongPress;
	DISPLAY_COLOR_TYPE DisplayColorSuperLongPress;
	DISPLAY_COLOR_TYPE DisplayColorUltraLongPress;

	char Message1[CONFIG_MESSAGE_MAX_LEN + 1];
	char Message2[CONFIG_MESSAGE_MAX_LEN + 1];
	char Message3[CONFIG_MESSAGE_MAX_LEN + 1];
	char Message10[CONFIG_MESSAGE_MAX_LEN + 1];
	char Message11[CONFIG_MESSAGE_MAX_LEN + 1];

	char ProductId[CONFIG_PRODUCT_ID_MAX_LEN + 1];

	char CustomMessagePropertyName[CONFIG_PROPERTY_NAME_MAX_LEN + 1];
	bool CustomMessageEnable;
	char CustomMessageJson[CONFIG_CUSTOM_MESSAGE_JSON_MAX_LEN + 1];

	int ActionCount;

	char WiFiSSID[CONFIG_WIFI_SSID_MAX_LEN + 1];
	char WiFiPassword[CONFIG_WIFI_PASSWORD_MAX_LEN + 1];
	char TimeServer[CONFIG_TIME_SERVER_MAX_LEN + 1];

	char ScopeId[CONFIG_SCOPE_ID_MAX_LEN + 1];
	char DeviceId[CONFIG_DEVICE_ID_MAX_LEN + 1];
	char SasKey[CONFIG_SAS_KEY_MAX_LEN + 1];

	char IoTHubConnectionString[CONFIG_IOTHUB_CONNECTION_STRING_MAX_LEN + 1];

	char APmodeSSID[CONFIG_APMODE_SSID_MAX_LEN + 1];
	char APmodePassword[CONFIG_APMODE_PASSWORD_MAX_LEN + 1];

	uint8_t CheckSum[2];
};

extern CONFIG_TYPE Config;

void strncpy_w_zero(char* dest, const char* src, int destSize);

void ConfigResetFactorySettings();
void ConfigRead();
void ConfigWrite();
void ConfigPrint();
