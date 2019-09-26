#include <Arduino.h>
#include "../helper/Config.h"
#include "ActionSendMessage.h"

#include "../helper/AutoShutdown.h"
#include <ReButton.h>
#include <AZ3166WiFi.h>
#include <parson.h>
#include "../azureiot/ReButtonClient.h"
#include <SystemTime.h>

static String stringformat(const char* format, ...)
{
    va_list args;
    char buf[1024];

    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    return buf;
}

static int ActionToActionNum(ACTION_TYPE action)
{
    switch (action)
    {
  	case ACTION_1:
          return 1;
  	case ACTION_2:
          return 2;
  	case ACTION_3:
          return 3;
  	case ACTION_10:
          return 10;
  	case ACTION_11:
          return 11;
      default:
          return -1;
    }
}

static const char* ActionToMessage(ACTION_TYPE action)
{
    switch (action)
    {
	case ACTION_1:
        return Config.Message1;
	case ACTION_2:
        return Config.Message2;
	case ACTION_3:
        return Config.Message3;
	case ACTION_10:
        return Config.Message10;
	case ACTION_11:
        return Config.Message11;
    default:
        return "UNKNOWN";
    }
}

static String MakeMessageJsonString(ACTION_TYPE action)
{
    String payload = "{";
    payload += String("\"actionNum\":\"") + String(ActionToActionNum(action)) + String("\"");
	payload += String(",\"message\":\"") + ActionToMessage(action) + String("\"");
    payload += String(",\"batteryVoltage\":") + String(ReButton::ReadPowerSupplyVoltage());
	if (Config.CustomMessageEnable)
	{
		payload += stringformat(",%s", Config.CustomMessageJson);
	}
    payload += "}";

	return payload;
}

static String MakeReportJsonString()
{
	JSON_Value* data = json_value_init_object();
	if (strlen(Config.CustomMessagePropertyName) >= 1) json_object_set_boolean(json_object(data), Config.CustomMessagePropertyName, Config.CustomMessageEnable);
	String jsonString = json_serialize_to_string(data);
	json_value_free(data);

	return jsonString;
}

static bool DeviceTwinReceived = false;

static void DeviceTwinUpdateCallbackFunc(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payLoad, size_t size)
{
    Serial.println("DeviceTwinUpdateCallbackFunc()");

	JSON_Value* root_value = json_parse_string((const char*)payLoad);
	if (root_value == NULL)
    {
        Serial.println("failure calling json_parse_string");
		return;
    }
    Serial.printf("%s\n", json_serialize_to_string(root_value));

	JSON_Object* root_object = json_value_get_object(root_value);
	int customMessageEnable;
	if (strlen(Config.CustomMessagePropertyName) <= 0)
	{
		customMessageEnable = -1;
	}
	else
	{
		customMessageEnable = json_object_dotget_boolean(root_object, stringformat("desired.%s.value", Config.CustomMessagePropertyName).c_str());
	}

    json_value_free(root_value);

    switch (customMessageEnable)
    {
    case 1:
        Config.CustomMessageEnable = true;
        break;
    case 0:
        Config.CustomMessageEnable = false;
        break;
    }

	DeviceTwinReceived = true;
}

bool ActionSendMessage(ACTION_TYPE action)
{
    Serial.println("ActionSendMessage() : Enter");

	////////////////////
	// Update auto shutdown

	AutoShutdownUpdateStartTime();

	////////////////////
	// Connect Wi-Fi

	SetNTPHost(Config.TimeServer);

	Serial.println("ActionSendMessage() : Wi-Fi - Connecting....");
	if (strlen(Config.WiFiSSID) <= 0 && strlen(Config.WiFiPassword) <= 0) return false;
    if (WiFi.begin(Config.WiFiSSID, Config.WiFiPassword) != WL_CONNECTED) return false;

    while (WiFi.status() != WL_CONNECTED)
    {
		wait_ms(100);
    }

    IPAddress ip = WiFi.localIP();
    Serial.printf("ActionSendMessage() : IP address is %s.\n", ip.get_address());

  	////////////////////
  	// Initialize IoTHub client

	ReButtonClient client;
    if (!client.Connect(&DeviceTwinUpdateCallbackFunc))
    {
        Serial.println("ActionSendMessage() : IoT Hub Connection failed");
        return false;
    }

    ////////////////////
    // Make sure we are connected

	Serial.println("ActionSendMessage() : Wait for connected.");
    while (!client.IsConnected())
    {
		client.DoWork();
		wait_ms(100);
    }

    ////////////////////
    // Make sure we are received Twin Update

	Serial.println("ActionSendMessage() : Wait for DeviceTwin received.");
	while (!DeviceTwinReceived)
    {
		client.DoWork();
		wait_ms(100);
    }

	////////////////////
	// Send message

    String payload = MakeMessageJsonString(action);
    if (!client.SendMessageAsync(payload.c_str()))
	{
		Serial.println("ActionSendMessage() : SendEventAsync failed");
		return false;
	}

    while (!client.IsMessageSent())
    {
		client.DoWork();
		wait_ms(100);
    }

	while (!client.IsAllEventsSent())
    {
		client.DoWork();
		wait_ms(100);
    }

	////////////////////
	// Report status

	client.DeviceTwinReport(MakeReportJsonString().c_str());

    while (!client.IsDeviceTwinReported())
    {
		client.DoWork();
		wait_ms(100);
    }

	////////////////////
	// Disconnect IoTHub

	client.Disconnect();


    Serial.println("Complete");

    return true;
}
