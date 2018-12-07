#include <Arduino.h>
#include <ReButton.h>
#include "ReButtonClient.h"
#include <DevkitDPSClient.h>

static const char* GLOBAL_DEVICE_ENDPOINT = "global.azure-devices-provisioning.net";

static void ConnectionStateCallbackFunc(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* context)
{
	ReButtonClient* client = (ReButtonClient*)context;

	if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED && reason == IOTHUB_CLIENT_CONNECTION_OK)
	{
		Serial.println("ConnectionStateCallbackFunc() : Connected");
		client->SetConnected(true);
	}
	else
	{
		Serial.printf("ConnectionStateCallbackFunc() : Disconnected (result:%d reason:%d)\n", result, reason);
		client->SetConnected(false);
	}
}

static void SendEventCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* context)
{
	ReButtonClient* client = (ReButtonClient*)context;

	if (result == IOTHUB_CLIENT_CONFIRMATION_OK)
	{
		Serial.println("SendEventCallback() : Message sent to Azure IoT Hub");
		client->SetMessageSent();
	}
	else
	{
		Serial.println("SendEventCallback() : Failed to send message to Azure IoT Hub");
	}

	client->DestroyMessage();
}

static void DeviceTwinCallbackFunc(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payLoad, size_t size, void* context)
{
	ReButtonClient* client = (ReButtonClient*)context;

	Serial.printf("DeviceTwinCallbackFunc() \n%.*s\n", size, payLoad);

	client->DeviceTwinUpdateCallbackInvoke(update_state, payLoad, size);
}

static void DeviceTwinReportCallbackFunc(int result, void* context)
{
	ReButtonClient* client = (ReButtonClient*)context;

	Serial.printf("DeviceTwinReportCallbackFunc() : %d\n", result);
	client->SetDeviceTwinReported();
}

ReButtonClient::ReButtonClient()
{
	_ClientHandle = NULL;
	_MessageHandle = NULL;
	_DeviceTwinUpdateCallbackFunc = NULL;
	_Connected = false;
	_MessageSent = false;
	_DeviceTwinReported = false;
}

ReButtonClient::~ReButtonClient()
{
	Disconnect();
}

void ReButtonClient::DoWork()
{
	IoTHubClient_LL_DoWork(_ClientHandle);
}

bool ReButtonClient::IsAllEventsSent()
{
	IOTHUB_CLIENT_STATUS status;
	if ((IoTHubClient_LL_GetSendStatus(_ClientHandle, &status) == IOTHUB_CLIENT_OK) && (status == IOTHUB_CLIENT_SEND_STATUS_BUSY))
	{
		return false;
	}
	else
	{
		return true;
	}
}

void ReButtonClient::DeviceTwinUpdateCallbackInvoke(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payLoad, size_t size)
{
	if (_DeviceTwinUpdateCallbackFunc != NULL) _DeviceTwinUpdateCallbackFunc(update_state, payLoad, size);
}

bool ReButtonClient::Connect(DeviceTwinUpdateCallback callback)
{
	_DeviceTwinUpdateCallbackFunc = callback;

	if (platform_init() != 0)
	{
		Serial.println("Failed to initialize the platform.");
		return false;
	}

	if (strlen(Config.IoTHubConnectionString) <= 0 && strlen(Config.ScopeId) >= 1 && strlen(Config.DeviceId) >= 1 && strlen(Config.SasKey) >= 1)
	{
		Serial.println("Device provisioning...");

		char uds[strlen(Config.SasKey) + 1];
		strcpy(uds, Config.SasKey);

		DevkitDPSSetLogTrace(false);
		DevkitDPSSetAuthType(DPS_AUTH_SYMMETRIC_KEY);
		if (!DevkitDPSClientStart(GLOBAL_DEVICE_ENDPOINT, Config.ScopeId, Config.DeviceId, uds, NULL, 0))
		{
			Serial.println("DPS client for GroupSAS has failed.");
			return false;
		}

		String iotHubConnectionString;
		iotHubConnectionString += "HostName=";
		iotHubConnectionString += DevkitDPSGetIoTHubURI();
		iotHubConnectionString += ";DeviceId=";
		iotHubConnectionString += DevkitDPSGetDeviceID();
		iotHubConnectionString += ";SharedAccessKey=";
		iotHubConnectionString += Config.SasKey;

		strncpy(Config.IoTHubConnectionString, iotHubConnectionString.c_str(), sizeof(Config.IoTHubConnectionString) - 1);
		Config.IoTHubConnectionString[sizeof(Config.IoTHubConnectionString) - 1] = '\0';
		ConfigWrite();
	}

	_ClientHandle = IoTHubClient_LL_CreateFromConnectionString(Config.IoTHubConnectionString, MQTT_Protocol);
	if (_ClientHandle == NULL)
	{
		Serial.println("ERROR: iotHubClientHandle is NULL!");
		return false;
	}

	// set options
	// https://github.com/Azure/azure-iot-sdk-c/blob/master/doc/Iothub_sdk_options.md

	if (IoTHubClient_LL_SetOption(_ClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
	{
		Serial.println("Failed to set option \"TrustedCerts\"");
		return false;
	}

	bool bLog = true;
	if (IoTHubClient_LL_SetOption(_ClientHandle, "logtrace", &bLog) != IOTHUB_CLIENT_OK)
	{
		Serial.println("Failed to set option \"logtrace\"");
		return false;
	}

	int connect_timeout = 30;
	if (IoTHubClient_LL_SetOption(_ClientHandle, "connect_timeout", &connect_timeout) != IOTHUB_CLIENT_OK)
	{
		Serial.println("Failed to set option \"TrustedCerts\"");
		return false;
	}

	int keepAlive = 10;
	if (IoTHubClient_LL_SetOption(_ClientHandle, "keepalive", &keepAlive) != IOTHUB_CLIENT_OK)
	{
		Serial.println("Failed to set option \"keepalive\"");
		return false;
	}

	if (IoTHubClient_LL_SetOption(_ClientHandle, "product_info", "ReButton") != IOTHUB_CLIENT_OK)
	{
		Serial.println("Failed to set option \"product_info\"");
		return false;
	}

	// Connection status change callback
	if (IoTHubClient_LL_SetConnectionStatusCallback(_ClientHandle, ConnectionStateCallbackFunc, this) != IOTHUB_CLIENT_OK)
	{
		Serial.println("IoTHubClient_LL_SetConnectionStatusCallback failed");
		return false;
	}

	if (IoTHubClient_LL_SetDeviceTwinCallback(_ClientHandle, DeviceTwinCallbackFunc, this) != IOTHUB_CLIENT_OK)
	{
		Serial.printf("IoTHubClient_LL_SetDeviceTwinCallback failed");
		return false;
	}

	Serial.println("InitializeIoTHubConnection() : Exit");
	return true;
}

void ReButtonClient::Disconnect()
{
	if (_ClientHandle != NULL)
	{
		IoTHubClient_LL_Destroy(_ClientHandle);
	}
}

bool ReButtonClient::IsConnected() const
{
	return _Connected;
}

void ReButtonClient::SetConnected(bool connect)
{
	_Connected = connect;
}

bool ReButtonClient::SendMessageAsync(const char* payload)
{
	if (_MessageHandle != NULL) return false;

	_MessageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)payload, strlen(payload));
	if (_MessageHandle == NULL)
	{
		Serial.println("SendMessageAsync() : Message Handle = NULL!");
		return false;
	}

	//MAP_HANDLE propMap = IoTHubMessage_Properties(_MessageHandle);
	//time_t now_utc = time(NULL); // utc time
	//MAP_RESULT res = Map_AddOrUpdate(propMap, "timestamp", ctime(&now_utc));
	//if (res != MAP_OK)
	//{
	//	Serial.println("Adding timestampe property failed");
	//}

	Serial.printf("SendMessageAsync() \n%s\r\n", payload);
	if (IoTHubClient_LL_SendEventAsync(_ClientHandle, _MessageHandle, SendEventCallback, this) != IOTHUB_CLIENT_OK)
	{
		DestroyMessage();
		Serial.println("SendMessageAsync() : Failed to hand over the message to IoTHubClient.");
		return false;
	}
	Serial.println("SendMessageAsync() : IoTHubClient accepted the message for delivery.");

	return true;
}

bool ReButtonClient::IsMessageSent() const
{
	return _MessageSent;
}

void ReButtonClient::SetMessageSent()
{
	_MessageSent = true;
}

void ReButtonClient::DestroyMessage()
{
	IoTHubMessage_Destroy(_MessageHandle);
	_MessageHandle = NULL;
}

bool ReButtonClient::DeviceTwinReport(const char* jsonString)
{
	_DeviceTwinReported = false;

	if (IoTHubClient_LL_SendReportedState(_ClientHandle, (unsigned char*)jsonString, strlen(jsonString), DeviceTwinReportCallbackFunc, this) != IOTHUB_CLIENT_OK)
	{
		Serial.println("DeviceTwinReport() : Failed");
		return false;
	}

	return true;
}

bool ReButtonClient::IsDeviceTwinReported() const
{
	return _DeviceTwinReported;
}

void ReButtonClient::SetDeviceTwinReported()
{
	_DeviceTwinReported = true;
}
