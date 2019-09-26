#pragma once

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

void InputBegin();
void InputTask();
bool InputIsCapturing();
INPUT_TYPE InputGetCurrentValue();
INPUT_TYPE InputGetConfirmValue();
const char* InputGetInputString(INPUT_TYPE value);
