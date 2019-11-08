#include <Arduino.h>
#include "../Common.h"
#include "ActionSendMessage.h"

#include <emw10xx-driver/EMW10xxInterface.h>
#include <system/SystemWiFi.h>
#include <system/SystemTime.h>
#include <azure_c_shared_utility/platform.h>
#include <IPAddress.h>

#include <ReButton.h>
#include "../azureiot/ReButtonClient2.h"

#define KEEP_ALIVE			(60)
#define LOG_TRACE			(false)

#define WIFI				((EMW10xxInterface*)WiFiInterface())
#define POLLING_INTERVAL	(100)

//static String MakeMessageJsonString(ACTION_TYPE action)
//{
//    String payload = "{";
//////    payload += String("\"actionNum\":\"") + String(ActionToActionNum(action)) + String("\"");
//////	payload += String(",\"message\":\"") + ActionToMessage(action) + String("\"");
//////	switch (action)
//////	{
//////	case ACTION_1:
//////		payload += String(",\"singleClick\":\"") + ActionToMessage(action) + String("\"");
//////		break;
//////	case ACTION_2:
//////		payload += String(",\"doubleClick\":\"") + ActionToMessage(action) + String("\"");
//////		break;
//////	case ACTION_3:
//////		payload += String(",\"tripleClick\":\"") + ActionToMessage(action) + String("\"");
//////		break;
//////	case ACTION_10:
//////		payload += String(",\"longPress\":\"") + ActionToMessage(action) + String("\"");
//////		break;
//////	case ACTION_11:
//////		payload += String(",\"superLongPress\":\"") + ActionToMessage(action) + String("\"");
//////		break;
//////	}
//////	payload += String(",\"batteryVoltage\":") + String(ReButton::ReadPowerSupplyVoltage());
//////	payload += String(",\"actionCount\":") + String(Config.ActionCount);
//	if (Config.CustomMessageEnable)
//	{
//		payload += stringformat(",%s", Config.CustomMessageJson);
//	}
//    payload += "}";
//
//	return payload;
//}
//
//static String MakeReportJsonString()
//{
//	JSON_Value* data = json_value_init_object();
//	if (strlen(Config.CustomMessagePropertyName) >= 1) json_object_set_boolean(json_object(data), Config.CustomMessagePropertyName, Config.CustomMessageEnable);
//	json_object_set_number(json_object(data), "actionCount", Config.ActionCount);
//	String jsonString = json_serialize_to_string(data);
//	json_value_free(data);
//
//	return jsonString;
//}
//
//static bool DeviceTwinReceived = false;
//
//static void DeviceTwinUpdateCallbackFunc(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payLoad, size_t size)
//{
//    Serial.println("DeviceTwinUpdateCallbackFunc()");
//
//	JSON_Value* root_value = json_parse_string((const char*)payLoad);
//	if (root_value == NULL)
//    {
//        Serial.println("failure calling json_parse_string");
//		return;
//    }
//    Serial.printf("%s\n", json_serialize_to_string(root_value));
//
//	JSON_Object* root_object = json_value_get_object(root_value);
//	int customMessageEnable;
//	if (strlen(Config.CustomMessagePropertyName) <= 0)
//	{
//		customMessageEnable = -1;
//	}
//	else
//	{
//		customMessageEnable = json_object_dotget_boolean(root_object, stringformat("desired.%s.value", Config.CustomMessagePropertyName).c_str());
//	}
//	int actionCount = json_object_dotget_number(root_object, "reported.actionCount");
//    json_value_free(root_value);
//
//    switch (customMessageEnable)
//    {
//    case 1:
//        Config.CustomMessageEnable = true;
//        break;
//    case 0:
//        Config.CustomMessageEnable = false;
//        break;
//    }
//
//	Config.ActionCount = actionCount;
//
//	DeviceTwinReceived = true;
//}

bool ActionSendMessage(ACTION_TYPE action)
{
    Serial.println("ActionSendMessage() : Enter");

	////////////////////
	// Update auto shutdown

	AutoShutdownUpdateStartTime();

	////////////////////
	// Connect Wi-Fi

	Serial.println("ActionSendMessage() : Wi-Fi - Connecting....");
	if (strlen(Config.WiFiSSID) <= 0 && strlen(Config.WiFiPassword) <= 0) return false;

	SetNTPHost(Config.TimeServer);
	InitSystemWiFi();
	WIFI->set_interface(Station);
	if (WIFI->connect(Config.WiFiSSID, Config.WiFiPassword, NSAPI_SECURITY_WPA_WPA2, 0) != 0) return false;
	platform_init();

	IPAddress ip;
	ip.fromString(WIFI->get_ip_address());
	Serial.printf("ActionSendMessage() : IP address is %s.\n", ip.get_address());

	////////////////////
	// Connect Azure

	ReButtonClient2 client;
	client.SetProductId(Config.ProductId);
	client.SetKeepAlive(KEEP_ALIVE);
	client.SetLogTrace(LOG_TRACE);
	if (strlen(Config.IoTHubConnectionString) >= 1)
	{
		if (!client.ConnectIoTHub(Config.IoTHubConnectionString)) return false;
	}
	else if (strlen(Config.IoTHubConnectionString) <= 0 && strlen(Config.ScopeId) >= 1 && strlen(Config.DeviceId) >= 1 && strlen(Config.SasKey) >= 1)
	{
		if (!client.ConnectIoTHubWithDPS(GLOBAL_DEVICE_ENDPOINT, Config.ScopeId, Config.DeviceId, Config.SasKey)) return false;
	}
	else
	{
		return false;
	}

    ////////////////////
    // Make sure we are connected

	Serial.println("ActionSendMessage() : Wait for connected.");
    while (!client.IsConnected())
    {
		client.DoWork();
		wait_ms(POLLING_INTERVAL);
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

	Config.ActionCount++;

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

    Serial.println("ActionSendMessage() : Complete");

    return true;
}
