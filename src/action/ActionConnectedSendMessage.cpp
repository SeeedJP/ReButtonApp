#include <Arduino.h>
#include "../Common.h"
#include "ActionConnectedSendMessage.h"

#include <emw10xx-driver/EMW10xxInterface.h>
#include <system/SystemWiFi.h>
#include <system/SystemTime.h>
#include <azure_c_shared_utility/platform.h>
#include <IPAddress.h>

#include <ReButton.h>
#include "../input/Input.h"
#include "../azureiot/ReButtonClient2.h"

#define KEEP_ALIVE			(60)
#define LOG_TRACE			(false)

#define WIFI				((EMW10xxInterface*)WiFiInterface())
#define POLLING_INTERVAL	(100)

#define LOOP_WAIT_TIME		(10)	// [msec.]

static const DISPLAY_COLOR_TYPE COLOR_DISCONNECTED = { 255, 255, 255 };
static const DISPLAY_COLOR_TYPE COLOR_CONNECTED    = { 255, 255, 255 };

bool ActionConnectedSendMessage()
{
    Serial.println("ActionConnectedSendMessage() : Enter");

	////////////////////
	// Set display

	DisplayStartActionDisconnected(COLOR_DISCONNECTED);
	bool isConnected = false;

	////////////////////
	// Connect Wi-Fi

	Serial.println("ActionConnectedSendMessage() : Wi-Fi - Connecting....");
	if (strlen(Config.WiFiSSID) <= 0 && strlen(Config.WiFiPassword) <= 0) return false;

	SetNTPHost(Config.TimeServer);
	InitSystemWiFi();
	WIFI->set_interface(Station);
	if (WIFI->connect(Config.WiFiSSID, Config.WiFiPassword, NSAPI_SECURITY_WPA_WPA2, 0) != 0) return false;
	platform_init();

	IPAddress ip;
	ip.fromString(WIFI->get_ip_address());
	Serial.printf("ActionConnectedSendMessage() : IP address is %s.\n", ip.get_address());

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

	Serial.print("Wait for connected");
	while (!client.IsConnected())
	{
		Serial.print(".");
		client.DoWork();
		delay(POLLING_INTERVAL);
	}
	Serial.println();

	////////////////////
	// Suspend auto shutdown

	AutoShutdownSuspend();

	////////////////////
    // Do work

	Serial.println("Ready.");

	while (ReButton::IsJumperShort())
	{
		if (isConnected)
		{
			if (!client.IsConnected())
			{
				DisplayStartActionDisconnected(COLOR_DISCONNECTED);
				isConnected = false;
			}
		}
		else
		{
			if (client.IsConnected())
			{
				DisplayStartActionConnected(COLOR_CONNECTED);
				isConnected = true;
			}
		}

		if (ReButton::IsButtonPressed())
		{
			InputBegin();
			for (;;)
			{
				InputTask();
				if (!InputIsCapturing()) break;
				DisplayColor(InputToDisplayColor(InputGetCurrentValue()));
				client.DoWork();
				delay(LOOP_WAIT_TIME);
			}
			INPUT_TYPE input = InputGetConfirmValue();
			Serial.printf("Button is %s.\n", InputGetInputString(input));

			client.Action = InputToAction(input);
			client.ActionCount++;
			client.SendTelemetryActionAsync();

			DisplayStartActionConnected(isConnected ? COLOR_CONNECTED : COLOR_DISCONNECTED);
		}

		client.DoWork();
		delay(POLLING_INTERVAL);
	}

	client.Disconnect();

    Serial.println("ActionConnectedSendMessage() : Complete");

    return true;
}
