#include <Arduino.h>
#include "../helper/Config.h"
#include "ActionConnectedSendMessage.h"

#include <emw10xx-driver/EMW10xxInterface.h>
#include <system/SystemWiFi.h>
#include <system/SystemTime.h>
#include <azure_c_shared_utility/platform.h>
#include <IPAddress.h>
#include <AzureDeviceClient.h>	// https://github.com/matsujirushi/AzureDeviceClient

#include <ReButton.h>
#include "../helper/AutoShutdown.h"
#include "../input/Input.h"

#define KEEP_ALIVE        (60)
#define LOG_TRACE         (false)

#define WIFI              ((EMW10xxInterface*)WiFiInterface())
#define POLLING_INTERVAL  (100)

bool ActionConnectedSendMessage()
{
    Serial.println("ActionConnectedSendMessage() : Enter");

	////////////////////
	// Suspend auto shutdown

	AutoShutdownSuspend();

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

	AzureDeviceClient client;
	client.SetProductId(Config.ProductId);
	client.SetKeepAlive(KEEP_ALIVE);
	client.SetLogTrace(LOG_TRACE);
	if (!client.ConnectIoTHub(Config.IoTHubConnectionString)) return false;	// TODO Support IoT Central
	Serial.print("Wait for connected");
	while (!client.IsConnected())
	{
		Serial.print(".");
		client.DoWork();
		delay(POLLING_INTERVAL);
	}
	Serial.println();

    ////////////////////
    // Do work

	Serial.println("Ready.");

	while (client.IsConnected())
	{
		if (ReButton::IsButtonPressed())
		{
			InputBegin();
			for (;;)
			{
				InputTask();
				if (!InputIsCapturing()) break;
				delay(10);
			}
			INPUT_TYPE input = InputGetConfirmValue();
			Serial.printf("Button is %s.\n", InputGetInputString(input));

			JSON_Value* telemetryValue = json_value_init_object();
			JSON_Object* telemetryObject = json_value_get_object(telemetryValue);

			json_object_set_string(telemetryObject, "message", InputGetInputString(input));

			client.SendTelemetryAsync(telemetryObject);

			json_value_free(telemetryValue);

		}

		client.DoWork();
		delay(POLLING_INTERVAL);
	}

    Serial.println("ActionConnectedSendMessage() : Complete");

    return true;
}
