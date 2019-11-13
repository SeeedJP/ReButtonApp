#pragma once

#include <AzureDeviceClient.h>	// https://github.com/matsujirushi/AzureDeviceClient
#include <GroveDriverPack.h>	// https://github.com/SeeedJP/GroveDriverPack
#include "../Common.h"

class ReButtonClient2 : public AzureDeviceClient
{
private:
	GroveBoard _Board;
	GroveTempHumiSHT35 _GroveSHT35;
	GroveTempHumiSHT31 _GroveSHT31;

	bool _IsDeviceTwinReceived;

public:
	ACTION_TYPE Action;
	int ActionCount;
	bool CustomMessageEnable;
	int TelemetryInterval;

public:
	ReButtonClient2();

	bool ConnectIoTHubWithDPS(const char* endpoint, const char* scopeId, const char* deviceId, const char* sasKey);

	bool IsDeviceTwinReceived() const;
	void SendTelemetryActionAsync();
	void SendTelemetryEnvironmentAsync();

private:
	void ReceivedProperties(JSON_Object* reportedObject);
	void ReceivedSettings(JSON_Object* desiredObject, bool complete);

	void SendPropertyCustomMessageEnableAsync();
	void SendPropertyActionCountAsync();
	void SendPropertyTelemetryIntervalAsync();

protected:
	virtual void DeviceTwinReceived(JSON_Object* deviceTwinObject);
	virtual void DesiredPropertyReceived(JSON_Object* desiredObject);

};
