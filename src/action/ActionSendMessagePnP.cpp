#include <Arduino.h>
#include "../../Config.h"
#include "ActionSendMessagePnP.h"

#include "../../AutoShutdown.h"
#include <AZ3166WiFi.h>
#include <SystemTime.h>
#include <DevkitDPSClient.h>
#include <certs/certs.h>
#include "../gencode/pnp_device.h"
#include "../helper/SasToken.h"
#include "../gencode/ReButton_impl.h"
#include <ReButton.h>
#include "./Grove_BME280/Seeed_BME280.h"

static const char* CUSTOM_PROVISIONING_DATA =
	"{"
	"\"__iot:interfaces\":"
	"{"
	"\"CapabilityModelId\": \"" DEVICE_CAPABILITY_MODEL_URI "\" ,"
	"\"CapabilityModel\": \"{}\""
	"}"
	"}";

static const char* GLOBAL_DEVICE_ENDPOINT = "global.azure-devices-provisioning.net";

static const int PROVISIONING_TRY_NUMBER = 10;
static const int PROVISIONING_TRY_INTERVAL = 10000;

static ACTION_TYPE Action;

static BME280 bme280;

bool ActionSendMessagePnP(ACTION_TYPE action)
{
    Serial.println("ActionSendMessagePnP() : Enter");
	Action = action;

    ReButton_Digital_Twin_Work_Flag = ReButton_Telemetry_Flags | ReButton_Twin_Flags; 

    Serial.printf("ReButton_Digital_Twin_Work_Flag %08x\r\n", ReButton_Digital_Twin_Work_Flag);


	////////////////////
	// Update auto shutdown

	AutoShutdownUpdateStartTime();

	////////////////////
	// Connect Wi-Fi

	SetTimeServer(Config.TimeServer);

	Serial.println("ActionSendMessagePnP() : Wi-Fi - Connecting....");
	if (strlen(Config.WiFiSSID) <= 0 && strlen(Config.WiFiPassword) <= 0) return false;
	if (WiFi.begin(Config.WiFiSSID, Config.WiFiPassword) != WL_CONNECTED) return false;

	while (WiFi.status() != WL_CONNECTED)
	{
		wait_ms(100);
	}

	IPAddress ip = WiFi.localIP();
	Serial.printf("ActionSendMessagePnP() : IP address is %s.\n", ip.get_address());

	////////////////////
	// Generate IoT Hub Device Connection String

	String iotHubConnectionString;
	if (strlen(Config.IoTHubConnectionString) >= 1)
	{
		iotHubConnectionString = Config.IoTHubConnectionString;
	}
	else if (strlen(Config.ScopeId) >= 1 && strlen(Config.DeviceId) >= 1 && strlen(Config.SasKey) >= 1)
	{

		////////////////////
		// Generate SAS token

		Serial.println("ActionSendMessagePnP() : Generate SAS token");
		Serial.printf("Scopy ID %s\r\n", Config.ScopeId);
		Serial.printf("Sas Key  %s\r\n", Config.SasKey);
		Serial.printf("Dev ID   %s\r\n", Config.DeviceId);

		char* deviceSasKeyPtr;
		if (GenerateDeviceSasToken(Config.SasKey, Config.DeviceId, &deviceSasKeyPtr))
		{
			Serial.println("ActionSendMessagePnP() : Generate SAS token failed");
			return false;
		}
		String deviceSasKey(deviceSasKeyPtr);
		free(deviceSasKeyPtr);

		////////////////////
		// Provisioning

		Serial.println("ActionSendMessagePnP() : Provisioning");
		DevkitDPSSetLogTrace(false);
		DevkitDPSSetAuthType(DPS_AUTH_SYMMETRIC_KEY);
		for (int i = 0; ; i++)
		{
			if (DevkitDPSClientStart(GLOBAL_DEVICE_ENDPOINT, Config.ScopeId, Config.DeviceId, (char*)deviceSasKey.c_str(), NULL, 0, CUSTOM_PROVISIONING_DATA)) break;
			Serial.println("(ERROR)");
			if (i + 1 >= PROVISIONING_TRY_NUMBER)
			{
				Serial.println("ActionSendMessagePnP() : Provisioning failed");
				return false;
			}
			delay(PROVISIONING_TRY_INTERVAL);
		}
		iotHubConnectionString = "HostName=";
		iotHubConnectionString += DevkitDPSGetIoTHubURI();
		iotHubConnectionString += ";DeviceId=";
		iotHubConnectionString += DevkitDPSGetDeviceID();
		iotHubConnectionString += ";SharedAccessKey=";
		iotHubConnectionString += deviceSasKey.c_str();
	}
	else
	{
		return false;
	}

	////////////////////
	// Connect IoT Hub

	Serial.println("ActionSendMessagePnP() : Connect IoT Hub");
	if (pnp_device_initialize(iotHubConnectionString.c_str(), certificates) != 0)
	{
		Serial.println("ActionSendMessagePnP() : Connect IoT Hub failed");
		return false;
	}

	Serial.println("ActionSendMessagePnP() : Send telemetry");
	BatteryInterface_Telemetry_SendAll();
	PushButtonInterface_Telemetry_SendAll();

	if (bme280.init())
	{
		TempHumidSensorInterface_Telemetry_SendAll();
	}

	while (ReButton_Digital_Twin_Work_Flag != 0)
	{
		DigitalTwinClientHelper_Check();
	}

    Serial.printf("ReButton_Digital_Twin_Work_Flag %08x\r\n", ReButton_Digital_Twin_Work_Flag);

	////////////////////
	// Disconnect IoT Hub

	Serial.println("ActionSendMessagePnP() : Disconnect IoT Hub");
	pnp_device_close();

	Serial.println("Complete");

    return true;
}

double Battery_Telemetry_ReadBatteryVoltage()
{
	return ReButton::ReadPowerSupplyVoltage();
}

PUSHBUTTON_ACTIONNUM PushButton_Telemetry_ReadActionNum()
{
	switch (Action)
	{
	case ACTION_1:
		return PUSHBUTTON_ACTIONNUM_SingleClick;
	case ACTION_2:
		return PUSHBUTTON_ACTIONNUM_DoubleClick;
	case ACTION_3:
		return PUSHBUTTON_ACTIONNUM_TripleClick;
	case ACTION_10:
		return PUSHBUTTON_ACTIONNUM_LongPress;
	case ACTION_11:
		return PUSHBUTTON_ACTIONNUM_SuperLongPress;
	default:
		return PUSHBUTTON_ACTIONNUM_ERROR;
	}
}

char* PushButton_Telemetry_ReadMessage()
{
	switch (Action)
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

double TempHumidSensor_Telemetry_ReadTemperature()
{
	float temperature = bme280.getTemperature();
	Serial.printf("Temperature %f\r\n", temperature);
    return temperature;
}

double TempHumidSensor_Telemetry_ReadHumidity()
{
	float humidity = bme280.getHumidity();
	Serial.printf("Humidity %f\r\n", humidity);
    return humidity;
}
