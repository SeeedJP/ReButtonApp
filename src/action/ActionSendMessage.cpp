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
	while (!client.IsDeviceTwinReceived())
    {
		client.DoWork();
		wait_ms(POLLING_INTERVAL);
    }

	////////////////////
	// Send message

	Serial.println("ActionSendMessage() : Send message.");
	client.Action = action;
	client.ActionCount++;
	client.SendTelemetryActionAsync();

	////////////////////
	// Wait for sent

	Serial.println("ActionSendMessage() : Wait for sent.");
	while (client.GetSendTelemetryIncompleteCount() != 0 || client.GetUpdateReportedPropertyIncompleteCount() != 0)
    {
		client.DoWork();
		wait_ms(POLLING_INTERVAL);
    }

	////////////////////
	// Disconnect Azure

	client.Disconnect();

    Serial.println("ActionSendMessage() : Complete");

    return true;
}
