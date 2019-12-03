#include <Arduino.h>
#include "../Common.h"
#include "MyTimer.h"

MyTimer::MyTimer()
{
	startTime = 0;
}

void MyTimer::start()
{
	startTime = SystemTickCounterRead();
}

void MyTimer::reset()
{
	startTime = SystemTickCounterRead();
}

int MyTimer::read_ms()
{
	return SystemTickCounterRead() - startTime;
}
