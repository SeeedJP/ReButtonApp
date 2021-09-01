#include "AzureDeviceClient.h"
#include <stdlib.h>
#include <iothubtransportmqtt.h>
#include <iothub_client_options.h>
#if defined ARDUINO_ARCH_SEEEDJP_REBUTTON
#include <azureiotcerts.h>
#else
#include <certs.h>
#endif

class MessageContext
{
public:
	friend class AzureDeviceClient;

private:
	AzureDeviceClient* Client;
	IOTHUB_MESSAGE_HANDLE Handle;

private:
	MessageContext(AzureDeviceClient* client, const char* payload)
	{
		Client = client;
		Handle = IoTHubMessage_CreateFromString(payload);
	}

public:
	~MessageContext()
	{
		IoTHubMessage_Destroy(Handle);
	}

};

AzureDeviceClient::AzureDeviceClient()
{
	_ClientHandle = NULL;
	_Connected = false;

	_X509Certificate = NULL;
	_X509PrivateKey = NULL;
	_ProductId = NULL;
	_KeepAlive = 240;
	_LogTrace = false;

	_SendTelemetryIncompleteCount = 0;
	_UpdateReportedPropertyIncompleteCount = 0;
}

AzureDeviceClient::~AzureDeviceClient()
{
	Disconnect();
}

void AzureDeviceClient::SetX509Certificate(const char* certificate)
{
	_X509Certificate = certificate;
}

void AzureDeviceClient::SetX509PrivateKey(const char* privateKey)
{
	_X509PrivateKey = privateKey;
}

void AzureDeviceClient::SetProductId(const char* productId)
{
	_ProductId = productId;
}

void AzureDeviceClient::SetKeepAlive(int keepAlive)
{
	_KeepAlive = keepAlive;
}

void AzureDeviceClient::SetLogTrace(bool on)
{
	_LogTrace = on;
}

bool AzureDeviceClient::IsConnected() const
{
	return _Connected;
}

int AzureDeviceClient::GetSendTelemetryIncompleteCount() const
{
	return _SendTelemetryIncompleteCount;
}

int AzureDeviceClient::GetUpdateReportedPropertyIncompleteCount() const
{
	return _UpdateReportedPropertyIncompleteCount;
}

void AzureDeviceClient::DoWork()
{
	IoTHubClient_LL_DoWork(_ClientHandle);
}

bool AzureDeviceClient::ConnectIoTHub(const char* connectionString, const char* model_id)
{
	_ClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol);
	if (_ClientHandle == NULL) return false;

	// set options
	// https://github.com/Azure/azure-iot-sdk-c/blob/master/doc/Iothub_sdk_options.md

	if (_X509Certificate != NULL) if (IoTHubClient_LL_SetOption(_ClientHandle, OPTION_X509_CERT, _X509Certificate) != IOTHUB_CLIENT_OK) abort();
	if (_X509PrivateKey != NULL) if (IoTHubClient_LL_SetOption(_ClientHandle, OPTION_X509_PRIVATE_KEY, _X509PrivateKey) != IOTHUB_CLIENT_OK) abort();
	if (_ProductId != NULL) if (IoTHubClient_LL_SetOption(_ClientHandle, OPTION_PRODUCT_INFO, _ProductId) != IOTHUB_CLIENT_OK) abort();
	if (IoTHubClient_LL_SetOption(_ClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK) abort();
	if (IoTHubClient_LL_SetOption(_ClientHandle, OPTION_LOG_TRACE, &_LogTrace) != IOTHUB_CLIENT_OK) abort();
	if (IoTHubClient_LL_SetOption(_ClientHandle, OPTION_KEEP_ALIVE, &_KeepAlive) != IOTHUB_CLIENT_OK) abort();
	if (model_id != NULL) if (IoTHubClient_LL_SetOption(_ClientHandle, OPTION_MODEL_ID, model_id) != IOTHUB_CLIENT_OK) abort();

	if (IoTHubClient_LL_SetConnectionStatusCallback(_ClientHandle, ConnectionStateCallback, this) != IOTHUB_CLIENT_OK) abort();
	if (IoTHubClient_LL_SetDeviceTwinCallback(_ClientHandle, DeviceTwinCallback, this) != IOTHUB_CLIENT_OK) abort();
		
	return true;
}

void AzureDeviceClient::Disconnect()
{
	if (_ClientHandle != NULL)
	{
		IoTHubClient_LL_Destroy(_ClientHandle);
		_ClientHandle = NULL;
		_Connected = false;
	}
}

void AzureDeviceClient::SendTelemetryAsync(JSON_Object* telemetryObject)
{
	auto jsonString = json_serialize_to_string(json_object_get_wrapping_value(telemetryObject));
	auto messageContext = new MessageContext(this, jsonString);
	json_free_serialized_string(jsonString);
	if (messageContext == NULL) abort();
	if (messageContext->Handle == NULL) abort();

	if (IoTHubMessage_SetContentTypeSystemProperty(messageContext->Handle, "application%2fjson") != IOTHUB_MESSAGE_OK) abort();
	if (IoTHubMessage_SetContentEncodingSystemProperty(messageContext->Handle, "utf-8") != IOTHUB_MESSAGE_OK) abort();

	if (_ProductId != NULL)
	{
		auto propMap = IoTHubMessage_Properties(messageContext->Handle);
		if (Map_AddOrUpdate(propMap, "productId", _ProductId) != MAP_OK) abort();
	}

	if (IoTHubClient_LL_SendEventAsync(_ClientHandle, messageContext->Handle, SendEventCallback, messageContext) != IOTHUB_CLIENT_OK) abort();
	_SendTelemetryIncompleteCount++;
}

void AzureDeviceClient::SendEventCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* context)
{
	auto messageContext = (MessageContext*)context;

	messageContext->Client->_SendTelemetryIncompleteCount--;

	delete messageContext;
}

void AzureDeviceClient::UpdateReportedPropertyAsync(JSON_Object* reportedObject)
{
	auto jsonString = json_serialize_to_string(json_object_get_wrapping_value(reportedObject));
	auto ret = IoTHubClient_LL_SendReportedState(_ClientHandle, (unsigned char*)jsonString, strlen(jsonString), SendReportedCallback, this);
	json_free_serialized_string(jsonString);
	if (ret != IOTHUB_CLIENT_OK) abort();

	_UpdateReportedPropertyIncompleteCount++;
}

void AzureDeviceClient::SendReportedCallback(int result, void* context)
{
	auto client = (AzureDeviceClient*)context;

	client->_UpdateReportedPropertyIncompleteCount--;
}

void AzureDeviceClient::DeviceTwinReceived(JSON_Object* deviceTwinObject)
{
}

void AzureDeviceClient::DesiredPropertyReceived(JSON_Object* desiredObject)
{
}

void AzureDeviceClient::ConnectionStateCallback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* context)
{
	auto client = (AzureDeviceClient*)context;

	client->_Connected = result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED;
}

void AzureDeviceClient::DeviceTwinCallback(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payload, size_t size, void* context)
{
	auto client = (AzureDeviceClient*)context;

	char jsonString[size + 1];
	memcpy(jsonString, payload, size);
	jsonString[size] = '\0';

	JSON_Value* root_value = json_parse_string(jsonString);
	if (root_value == NULL) abort();
	JSON_Object* root_object = json_value_get_object(root_value);

	switch (update_state)
	{
	case DEVICE_TWIN_UPDATE_COMPLETE:
		client->DeviceTwinReceived(root_object);
		break;
	case DEVICE_TWIN_UPDATE_PARTIAL:
		client->DesiredPropertyReceived(root_object);
		break;
	default:
		abort();
	}

	json_value_free(root_value);
}
