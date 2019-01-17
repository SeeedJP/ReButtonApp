#include <Arduino.h>
#include "Config.h"
#include "ActionSendMessage.h"

#include "AutoShutdown.h"
#include <ReButton.h>
#include <AZ3166WiFi.h>
#include <parson.h>
#include "ReButtonClient.h"
#include <SystemTime.h>

class GroveUltrasonicRanger
{
private:
	DigitalInOut* _Pin;

	unsigned long MicrosDiff(unsigned long begin, unsigned long end);
	unsigned long PulseIn(int state, unsigned long timeout = 1000000);

public:
	float Distance;

public:
	GroveUltrasonicRanger(DigitalInOut* pin)
	{
		_Pin = pin;
	}

	void Init();
	void Read();

};

unsigned long GroveUltrasonicRanger::MicrosDiff(unsigned long begin, unsigned long end)
{
	return end - begin;
}

unsigned long GroveUltrasonicRanger::PulseIn(int state, unsigned long timeout)
{
	auto begin = micros();

	// wait for any previous pulse to end
	while (_Pin->read() == state) if (MicrosDiff(begin, micros()) >= timeout) return 0;

	// wait for the pulse to start
	while (_Pin->read() != state) if (MicrosDiff(begin, micros()) >= timeout) return 0;
	auto pulseBegin = micros();

	// wait for the pulse to stop
	while (_Pin->read() == state) if (MicrosDiff(begin, micros()) >= timeout) return 0;
	auto pulseEnd = micros();

	return MicrosDiff(pulseBegin, pulseEnd);
}

void GroveUltrasonicRanger::Init()
{
	_Pin->input();
	_Pin->write(0);
}

void GroveUltrasonicRanger::Read()
{
	_Pin->output();
	_Pin->write(1);
	wait_ms(10);

	_Pin->write(0);
	_Pin->input();

	auto duration = PulseIn(1);
	if (duration == 0)
	{
		Distance = 0;
		return;
	}

	Distance = (float)duration * 0.34f / 2.0f;
}

static String stringformat(const char* format, ...)
{
    va_list args;
    char buf[1024];

    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    return buf;
}

static int ActionToActionNum(ACTION_TYPE action)
{
    switch (action)
    {
  	case ACTION_1:
          return 1;
  	case ACTION_2:
          return 2;
  	case ACTION_3:
          return 3;
  	case ACTION_10:
          return 10;
  	case ACTION_11:
          return 11;
      default:
          return -1;
    }
}

static const char* ActionToMessage(ACTION_TYPE action)
{
    switch (action)
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

static String MakeMessageJsonString(ACTION_TYPE action, double distance)
{
    String payload = "{";
    payload += String("\"actionNum\":\"") + String(ActionToActionNum(action)) + String("\"");
	payload += String(",\"message\":\"") + ActionToMessage(action) + String("\"");
    payload += String(",\"batteryVoltage\":") + String(ReButton::ReadPowerSupplyVoltage());
	if (Config.CustomMessageEnable)
	{
		payload += stringformat(",\"customMessage\":{%s}", Config.CustomMessageJson);
	}
	payload += String(",\"distance\":") + String(distance);
	payload += "}";

	return payload;
}

static String MakeReportJsonString()
{
	JSON_Value* data = json_value_init_object();
	if (strlen(Config.CustomMessagePropertyName) >= 1) json_object_dotset_boolean(json_object(data), stringformat("%s.value", Config.CustomMessagePropertyName).c_str(), Config.CustomMessageEnable);
	//json_object_dotset_number(json_object(data), "batteryVoltage.value", ReButton::ReadPowerSupplyVoltage());
	String jsonString = json_serialize_to_string(data);
	json_value_free(data);

	return jsonString;
}

static bool DeviceTwinReceived = false;

static void DeviceTwinUpdateCallbackFunc(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payLoad, size_t size)
{
    Serial.println("DeviceTwinUpdateCallbackFunc()");

	JSON_Value* root_value = json_parse_string((const char*)payLoad);
	if (root_value == NULL)
    {
        Serial.println("failure calling json_parse_string");
		return;
    }
    Serial.printf("%s\n", json_serialize_to_string(root_value));

	JSON_Object* root_object = json_value_get_object(root_value);
	int customMessageEnable;
	if (strlen(Config.CustomMessagePropertyName) <= 0)
	{
		customMessageEnable = -1;
	}
	else
	{
		customMessageEnable = json_object_dotget_boolean(root_object, stringformat("desired.%s.value", Config.CustomMessagePropertyName).c_str());
	}

    json_value_free(root_value);

    switch (customMessageEnable)
    {
    case 1:
        Config.CustomMessageEnable = true;
        break;
    case 0:
        Config.CustomMessageEnable = false;
        break;
    }

	DeviceTwinReceived = true;
}

bool ActionSendMessage(ACTION_TYPE action)
{
    Serial.println("ActionSendMessage() : Enter");

	////////////////////
	// Update auto shutdown

	AutoShutdownUpdateStartTime();

	////////////////////
	// Connect Wi-Fi

	SetNTPHost(Config.TimeServer);

	Serial.println("ActionSendMessage() : Wi-Fi - Connecting....");
	if (strlen(Config.WiFiSSID) <= 0 && strlen(Config.WiFiPassword) <= 0) return false;
    if (WiFi.begin(Config.WiFiSSID, Config.WiFiPassword) != WL_CONNECTED) return false;

    while (WiFi.status() != WL_CONNECTED)
    {
		wait_ms(100);
    }

    IPAddress ip = WiFi.localIP();
    Serial.printf("ActionSendMessage() : IP address is %s.\n", ip.get_address());

  	////////////////////
  	// Initialize IoTHub client

	ReButtonClient client;
    if (!client.Connect(&DeviceTwinUpdateCallbackFunc))
    {
        Serial.println("ActionSendMessage() : IoT Hub Connection failed");
        return false;
    }

    ////////////////////
    // Make sure we are connected

	Serial.println("ActionSendMessage() : Wait for connected.");
    while (!client.IsConnected())
    {
		client.DoWork();
		wait_ms(100);
    }

    ////////////////////
    // Make sure we are received Twin Update

	Serial.println("ActionSendMessage() : Wait for DeviceTwin received.");
	while (!DeviceTwinReceived)
    {
		client.DoWork();
		wait_ms(100);
    }

	////////////////////
	// Read sensor value

	//I2C i2c(PB_9, PB_8);

	//char data[6];
	//data[0] = 0x2d;
	//data[1] = 0x08;
	//i2c.write(0x53 << 1, data, 2);

	//for (;;)
	//{
	//	data[0] = 0x32;
	//	i2c.write(0x53 << 1, data, 1);
	//	i2c.read(0x53 << 1, data, 6);
	//	int16_t val;
	//	((uint8_t*)&val)[0] = data[0];
	//	((uint8_t*)&val)[1] = data[1];
	//	float x = val * 2.0 / 512;
	//	((uint8_t*)&val)[0] = data[2];
	//	((uint8_t*)&val)[1] = data[3];
	//	float y = val * 2.0 / 512;
	//	((uint8_t*)&val)[0] = data[4];
	//	((uint8_t*)&val)[1] = data[5];
	//	float z = val * 2.0 / 512;

	//	Serial.printf("%.2f, %.2f, %.2f\n", x, y, z);

	//	delay(100);
	//}

	DigitalInOut pin(PB_8);
	GroveUltrasonicRanger ranger(&pin);
	ranger.Init();
	float distance = 0;
	for (int i = 0; i < 5; i++)
	{
		ranger.Read();
		distance += ranger.Distance;
		Serial.printf("Ranger = %.0f[mm]\n", ranger.Distance);
		wait_ms(100);
	}
	distance /= 5;

	////////////////////
	// Send message

    String payload = MakeMessageJsonString(action, distance);
    if (!client.SendMessageAsync(payload.c_str()))
	{
		Serial.println("ActionSendMessage() : SendEventAsync failed");
		return false;
	}

    while (!client.IsMessageSent())
    {
		client.DoWork();
		wait_ms(100);
    }

	while (!client.IsAllEventsSent())
    {
		client.DoWork();
		wait_ms(100);
    }

	////////////////////
	// Report status

	client.DeviceTwinReport(MakeReportJsonString().c_str());

    while (!client.IsDeviceTwinReported())
    {
		client.DoWork();
		wait_ms(100);
    }

	////////////////////
	// Disconnect IoTHub

	client.Disconnect();


    Serial.println("Complete");

    return true;
}
