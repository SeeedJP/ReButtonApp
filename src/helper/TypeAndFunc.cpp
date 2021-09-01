#include <Arduino.h>
#include "../Common.h"
#include "TypeAndFunc.h"

const char CONFIG_FIRMWARE_VERSION[] = "1.11";

const DISPLAY_COLOR_TYPE DISPLAY_OFF   = { 0  , 0  , 0 };	// OFF
const DISPLAY_COLOR_TYPE DISPLAY_OK    = { 0  , 255, 0 };	// GREEN
const DISPLAY_COLOR_TYPE DISPLAY_ERROR = { 255, 0  , 0 };	// RED

const int INPUT_WAIT_FOR_CLICK_TIME   = 200;	// [msec.]
const int INPUT_LONG_PRESS_TIME       = 3000;	// [msec.]
const int INPUT_SUPER_LONG_PRESS_TIME = 6000;	// [msec.]
const int INPUT_ULTRA_LONG_PRESS_TIME = 10000;	// [msec.]

const char GLOBAL_DEVICE_ENDPOINT[] = "global.azure-devices-provisioning.net";
const char MODEL_ID[] = "dtmi:seeedkk:rebutton;1";

const char SSID_PREFIX[] = "AZB-";

ACTION_TYPE InputToAction(INPUT_TYPE value)
{
	switch (value)
	{
	case INPUT_NONE:
		return ACTION_CONNECTED;
	case INPUT_SINGLE_CLICK:
		return ACTION_1;
	case INPUT_DOUBLE_CLICK:
		return ACTION_2;
	case INPUT_TRIPLE_CLICK:
		return ACTION_3;
	case INPUT_LONG_PRESS:
		return ACTION_10;
	case INPUT_SUPER_LONG_PRESS:
		return ACTION_11;
	case INPUT_ULTRA_LONG_PRESS:
		return ACTION_AP;
	default:
		return ACTION_NONE;
	}
}

const char* InputGetInputString(INPUT_TYPE value)
{
	switch (value)
	{
	case INPUT_CAPTURING:
		return "INPUT_CAPTURING";
	case INPUT_NONE:
		return "INPUT_NONE";
	case INPUT_SINGLE_CLICK:
		return "INPUT_SINGLE_CLICK";
	case INPUT_DOUBLE_CLICK:
		return "INPUT_DOUBLE_CLICK";
	case INPUT_TRIPLE_CLICK:
		return "INPUT_TRIPLE_CLICK";
	case INPUT_LONG_PRESS:
		return "INPUT_LONG_PRESS";
	case INPUT_SUPER_LONG_PRESS:
		return "INPUT_SUPER_LONG_PRESS";
	case INPUT_ULTRA_LONG_PRESS:
		return "INPUT_ULTRA_LONG_PRESS";
	default:
		return "UNKNOWN";
	}
}

DISPLAY_COLOR_TYPE InputToDisplayColor(INPUT_TYPE value)
{
	switch (value)
	{
	case INPUT_NONE:
		return DISPLAY_OFF;
	case INPUT_SINGLE_CLICK:
		return Config.DisplayColorSingleClick;
	case INPUT_DOUBLE_CLICK:
		return Config.DisplayColorDoubleClick;
	case INPUT_TRIPLE_CLICK:
		return Config.DisplayColorTripleClick;
	case INPUT_LONG_PRESS:
		return Config.DisplayColorLongPress;
	case INPUT_SUPER_LONG_PRESS:
		return Config.DisplayColorSuperLongPress;
	case INPUT_ULTRA_LONG_PRESS:
		return Config.DisplayColorUltraLongPress;
	default:
		return DISPLAY_ERROR;
	}
}

const char* ActionGetActionString(ACTION_TYPE value)
{
	switch (value)
	{
	case ACTION_1:
		return "ACTION_1";
	case ACTION_2:
		return "ACTION_2";
	case ACTION_3:
		return "ACTION_3";
	case ACTION_10:
		return "ACTION_10";
	case ACTION_11:
		return "ACTION_11";
	case ACTION_CONNECTED:
		return "ACTION_CONNECTED";
	case ACTION_AP:
		return "ACTION_AP";
	default:
		return "UNKNOWN";
	}
}

int ActionToActionNum(ACTION_TYPE action)
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

const char* ActionToMessage(ACTION_TYPE action)
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

void strncpy_w_zero(char* dest, const char* src, int destSize)
{
	strncpy(dest, src, destSize - 1);
	dest[destSize - 1] = '\0';
}

String stringformat(const char* format, ...)
{
	va_list args;
	char buf[1024];

	va_start(args, format);
	vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);

	return buf;
}

String stringformat2(int maxLength, const char* format, ...)
{
	va_list args;
	char* buf = new char[maxLength + 1];

	va_start(args, format);
	vsnprintf(buf, maxLength + 1, format, args);
	va_end(args);

	String str(buf);

	delete[] buf;

	return str;
}
