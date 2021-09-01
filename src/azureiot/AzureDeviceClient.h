#pragma once

#include <iothub_client_ll.h>
#include <parson.h>

class AzureDeviceClient
{
private:
	IOTHUB_CLIENT_LL_HANDLE _ClientHandle;
	bool _Connected;
	const char* _X509Certificate;
	const char* _X509PrivateKey;
	const char* _ProductId;
	int _KeepAlive;
	bool _LogTrace;
	int _SendTelemetryIncompleteCount;
	int _UpdateReportedPropertyIncompleteCount;

	static void ConnectionStateCallback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* context);
	static void DeviceTwinCallback(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payload, size_t size, void* context);
	static void SendEventCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* context);
	static void SendReportedCallback(int result, void* context);

public:
	AzureDeviceClient();
	virtual ~AzureDeviceClient();

	void SetX509Certificate(const char* certificate);
	void SetX509PrivateKey(const char* privateKey);
	void SetProductId(const char* productId);
	void SetKeepAlive(int keepAlive);
	void SetLogTrace(bool on);

	bool IsConnected() const;
	int GetSendTelemetryIncompleteCount() const;
	int GetUpdateReportedPropertyIncompleteCount() const;

	void DoWork();

	bool ConnectIoTHub(const char* connectionString, const char* model_id = NULL);
	void Disconnect();

	void SendTelemetryAsync(JSON_Object* telemetryObject);
	void UpdateReportedPropertyAsync(JSON_Object* reportedObject);

protected:
	virtual void DeviceTwinReceived(JSON_Object* deviceTwinObject);
	virtual void DesiredPropertyReceived(JSON_Object* desiredObject);

};
