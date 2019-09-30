#pragma once

#include <AzureDeviceClient.h>	// https://github.com/matsujirushi/AzureDeviceClient
#include "../Common.h"

class ReButtonClient2 : public AzureDeviceClient
{
private:
	bool _IsDeviceTwinReceived;

public:
	int ReportedActionCount;

public:
	ReButtonClient2();

	bool ConnectIoTHubWithDPS(const char* endpoint, const char* scopeId, const char* deviceId, const char* sasKey);

	void ReceivedProperties(JSON_Object* reportedObject);
	void SendPropertyActionCountAsync();

	bool ReceivedSettings(JSON_Object* desiredObject);

	void SendTelemetryAsync(ACTION_TYPE action);

protected:
	virtual void DeviceTwinReceived(JSON_Object* deviceTwinObject);
	virtual void DesiredPropertyReceived(JSON_Object* desiredObject);

};
