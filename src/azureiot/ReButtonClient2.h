#pragma once

#include <AzureDeviceClient.h>	// https://github.com/matsujirushi/AzureDeviceClient
#include "../Common.h"

class ReButtonClient2 : public AzureDeviceClient
{
private:
	bool _IsDeviceTwinReceived;

public:
	ACTION_TYPE Action;
	int ReportedActionCount;

public:
	ReButtonClient2();

	bool ConnectIoTHubWithDPS(const char* endpoint, const char* scopeId, const char* deviceId, const char* sasKey);

	bool IsDeviceTwinReceived() const;
	void SendTelemetryActionAsync();

private:
	void ReceivedProperties(JSON_Object* reportedObject);
	void ReceivedSettings(JSON_Object* desiredObject, bool complete);

	void SendPropertyCustomMessageEnableAsync();
	void SendPropertyActionCountAsync();

protected:
	virtual void DeviceTwinReceived(JSON_Object* deviceTwinObject);
	virtual void DesiredPropertyReceived(JSON_Object* desiredObject);

};
