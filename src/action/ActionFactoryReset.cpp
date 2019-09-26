#include <Arduino.h>
#include "../helper/Config.h"
#include "ActionFactoryReset.h"

bool ActionFactoryReset()
{
	Serial.println("ActionFactoryReset()");

	ConfigResetFactorySettings();
	ConfigWrite();

	return true;
}
