#include <Arduino.h>
#include "GroveTempHumiSHT31.h"

#define I2C_ADDRESS			(0x44)

#define POLYNOMIAL			(0x31)

#define CMD_SOFT_RESET		(0x30a2)
#define CMD_SINGLE_HIGH		(0x2400)

void GroveTempHumiSHT31::SendCommand(uint16_t cmd)
{
	uint8_t writeData[2];
	writeData[0] = cmd >> 8;
	writeData[1] = cmd & 0xff;
	_Device->write(I2C_ADDRESS << 1, (const char*)writeData, sizeof(writeData));
}

uint8_t GroveTempHumiSHT31::CalcCRC8(const uint8_t* data, int dataSize)
{
	uint8_t crc = 0xff;

	for (int j = dataSize; j > 0; j--)
	{
		crc ^= *data++;

		for (int i = 8; i > 0; i--)
		{
			crc = crc & 0x80 ? crc << 1 ^ POLYNOMIAL : crc << 1;
		}
	}

	return crc;
}

void GroveTempHumiSHT31::Init()
{
	SendCommand(CMD_SOFT_RESET);
	wait_ms(1);
}

void GroveTempHumiSHT31::Read()
{
	SendCommand(CMD_SINGLE_HIGH);
	wait_ms(15);

	uint8_t readData[6];
	_Device->read(I2C_ADDRESS << 1, (char*)readData, sizeof(readData));

	Temperature = 0;
	Humidity = 0;
	if (readData[2] != CalcCRC8(&readData[0], 2)) return;
	if (readData[5] != CalcCRC8(&readData[3], 2)) return;

	uint16_t ST;
	ST = readData[0];
	ST <<= 8;
	ST |= readData[1];

	uint16_t SRH;
	SRH = readData[3];
	SRH <<= 8;
	SRH |= readData[4];

	Temperature = (float)ST * 175 / 0xffff - 45;
	Humidity = (float)SRH * 100 / 0xffff;
}
