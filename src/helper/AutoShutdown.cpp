#include <Arduino.h>
#include "../Common.h"
#include "AutoShutdown.h"

#include <Thread.h>
#include <ReButton.h>

static Thread AutoShutdownThread(osPriorityHigh);
static Mutex StartTimeMutex;
static volatile unsigned long StartTime;
static Mutex AutoShutdownTimeoutMutex;
static volatile int AutoShutdownTimeout;
static Mutex AutoShutdownBlocking;

void AutoShutdownUpdateStartTime()
{
	StartTimeMutex.lock();
	StartTime = millis();
	StartTimeMutex.unlock();
}

static unsigned long AutoShutdownGetStartTime()
{
	StartTimeMutex.lock();
	int startTime = StartTime;
	StartTimeMutex.unlock();
	return startTime;
}

static int AutoShutdownGetTimeout()
{
	AutoShutdownTimeoutMutex.lock();
	int timeout = AutoShutdownTimeout;
	AutoShutdownTimeoutMutex.unlock();
	return timeout;
}

void AutoShutdownSetTimeout(int timeout)
{
	AutoShutdownTimeoutMutex.lock();
	AutoShutdownTimeout = timeout;
	AutoShutdownTimeoutMutex.unlock();
}

static void AutoShutdownMain()
{
	AutoShutdownUpdateStartTime();

	for (;;)
	{
		while (millis() <= AutoShutdownGetStartTime() + AutoShutdownGetTimeout())
		{
			delay(100);
		}
		AutoShutdownBlocking.lock();
		if (millis() > AutoShutdownGetStartTime() + AutoShutdownGetTimeout()) break;
		AutoShutdownBlocking.unlock();
	}

	Serial.println("AUTO SHUTDOWN!");
	delay(100);
	for (;;)
	{
		ReButton::PowerSupplyEnable(false);
		delay(1000);
	}
}

void AutoShutdownBegin(int timeout)
{
	AutoShutdownSetTimeout(timeout);
	AutoShutdownThread.start(AutoShutdownMain);
}

void AutoShutdownSuspend()
{
	AutoShutdownBlocking.lock();
}

void AutoShutdownResume()
{
	AutoShutdownBlocking.unlock();
}
