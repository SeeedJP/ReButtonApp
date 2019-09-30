#include <Arduino.h>
#include "src/Common.h"

#include <ReButton.h>
#include "src/input/Input.h"
#include "src/action/Action.h"

#define LOOP_WAIT_TIME	(10)	// [msec.]
#define POWER_OFF_TIME	(1000)	// [msec.]

void setup()
{
	////////////////////
	// Setup auto shutdown

	AutoShutdownBegin(CONFIG_AUTO_SHUTDOWN_TIMEOUT);
	DisplayBegin();

	////////////////////
	// Read CONFIG

	ConfigRead();

	Serial.printf("Firmware version is %s.\n", CONFIG_FIRMWARE_VERSION);

	Serial.println("Parameters:");
	Serial.println("-----");
	ConfigPrint();
	Serial.println("-----");

	if (ReButton::IsButtonPressed() && ReButton::IsJumperShort())
	{
		Serial.println("Force factory reset.");
		ConfigResetFactorySettings();
		ConfigWrite();
		return;
	}

	////////////////////
	// INPUT

	InputBegin();
	for (;;)
	{
		InputTask();
		if (!InputIsCapturing()) break;
		DisplayColor(InputToDisplayColor(InputGetCurrentValue()));
		delay(LOOP_WAIT_TIME);
	}
	INPUT_TYPE input = InputGetConfirmValue();
	Serial.printf("Button is %s.\n", InputGetInputString(input));

	////////////////////
	// FLASH

	DisplayStartAction(InputToDisplayColor(input));

	////////////////////
	// ACTION

	ACTION_TYPE action = InputToAction(input);
	Serial.printf("Action is %s.\n", ActionGetActionString(action));

	if (!ActionTaskBlocking(action)) return;

	////////////////////
	// FINISH

	Serial.println("Finish.");

	DisplayStartFinish(InputToDisplayColor(input));
	delay(1500);

	////////////////////
	// Power off

	ReButton::PowerSupplyEnable(false);
	delay(POWER_OFF_TIME);
}

void loop()
{
	////////////////////
	// FINISH (Error)

	for (int i = 0; i < 3; i++)
	{
		DisplayColor(DISPLAY_ERROR);
		delay(200);
		DisplayColor(DISPLAY_OFF);
		delay(200);
	}
	ReButton::PowerSupplyEnable(false);
	delay(POWER_OFF_TIME);
}
