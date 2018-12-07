#include <Arduino.h>
#include <ReButton.h>
#include "AzureIotHub.h"
#include "Config.h"

typedef void (*DeviceTwinUpdateCallback)(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payLoad, size_t size);

class ReButtonClient
{
private:
	IOTHUB_CLIENT_LL_HANDLE _ClientHandle;
	IOTHUB_MESSAGE_HANDLE _MessageHandle;
	DeviceTwinUpdateCallback _DeviceTwinUpdateCallbackFunc;
	bool _Connected;
	bool _MessageSent;
	bool _DeviceTwinReported;

public:
	ReButtonClient();
	~ReButtonClient();

	void DoWork();
	bool IsAllEventsSent();
	void DeviceTwinUpdateCallbackInvoke(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payLoad, size_t size);

	bool Connect(DeviceTwinUpdateCallback callback);
	void Disconnect();
	bool IsConnected() const;
	void SetConnected(bool connect);

	bool SendMessageAsync(const char* payload);
	bool IsMessageSent() const;
	void SetMessageSent();
	void DestroyMessage();

	bool DeviceTwinReport(const char* jsonString);
	bool IsDeviceTwinReported() const;
	void SetDeviceTwinReported();

};
