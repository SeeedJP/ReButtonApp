#pragma once

class GroveTempHumiSHT31
{
private:
	I2C* _Device;

	void SendCommand(uint16_t cmd);
	static uint8_t CalcCRC8(const uint8_t* data, int dataSize);

public:
	float Temperature;
	float Humidity;

public:
	GroveTempHumiSHT31(I2C* i2c)
	{
		_Device = i2c;
	}

	void Init();
	void Read();

};
