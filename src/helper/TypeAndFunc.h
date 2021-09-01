#pragma once

struct DISPLAY_COLOR_TYPE
{
	uint8_t Red;
	uint8_t Green;
	uint8_t Blue;
};

enum INPUT_TYPE
{
	INPUT_CAPTURING,
	INPUT_NONE,
	INPUT_SINGLE_CLICK,
	INPUT_DOUBLE_CLICK,
	INPUT_TRIPLE_CLICK,
	INPUT_LONG_PRESS,
	INPUT_SUPER_LONG_PRESS,
	INPUT_ULTRA_LONG_PRESS,
};

enum ACTION_TYPE
{
	ACTION_NONE,
	ACTION_1,
	ACTION_2,
	ACTION_3,
	ACTION_10,
	ACTION_11,
	ACTION_CONNECTED,
	ACTION_AP,
};

extern const char CONFIG_FIRMWARE_VERSION[];

extern const DISPLAY_COLOR_TYPE DISPLAY_OFF;
extern const DISPLAY_COLOR_TYPE DISPLAY_OK;
extern const DISPLAY_COLOR_TYPE DISPLAY_ERROR;

extern const int INPUT_WAIT_FOR_CLICK_TIME;
extern const int INPUT_LONG_PRESS_TIME;
extern const int INPUT_SUPER_LONG_PRESS_TIME;
extern const int INPUT_ULTRA_LONG_PRESS_TIME;

extern const char GLOBAL_DEVICE_ENDPOINT[];
extern const char MODEL_ID[];

extern const char SSID_PREFIX[];

ACTION_TYPE InputToAction(INPUT_TYPE value);
const char* InputGetInputString(INPUT_TYPE value);
DISPLAY_COLOR_TYPE InputToDisplayColor(INPUT_TYPE value);

const char* ActionGetActionString(ACTION_TYPE value);
int ActionToActionNum(ACTION_TYPE action);
const char* ActionToMessage(ACTION_TYPE action);

void strncpy_w_zero(char* dest, const char* src, int destSize);
String stringformat(const char* format, ...);
String stringformat2(int maxLength, const char* format, ...);
