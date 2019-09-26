#include <Arduino.h>
#include "Input.h"
#include <ReButton.h>

static const int InputWaitForClickTime   = 200;	  // [msec.]
static const int InputLongPressTime      = 3000;  // [msec.]
static const int InputSuperLongPressTime = 6000;  // [msec.]
static const int InputUltraLongPressTime = 10000; // [msec.]

static INPUT_TYPE InputConfirm;
static INPUT_TYPE InputCurrent;

static bool ButtonState;
static Timer ButtonTimer;

void InputBegin()
{
	if (!ReButton::IsButtonPressed())
	{
		InputConfirm = INPUT_NONE;
		InputCurrent = INPUT_NONE;
		ButtonState = false;
	}
	else
	{
		InputConfirm = INPUT_CAPTURING;
		InputCurrent = INPUT_SINGLE_CLICK;
		ButtonState = true;
	}

	ButtonTimer.reset();
	ButtonTimer.start();
}

void InputTask()
{
	bool preButtonState = ButtonState;
	ButtonState = ReButton::IsButtonPressed();

	if (InputConfirm != INPUT_CAPTURING) return;

	// Pushing
	if (!preButtonState && ButtonState)
	{
		ButtonTimer.reset();
		ButtonTimer.start();
		switch (InputCurrent)
		{
		case INPUT_SINGLE_CLICK:
			InputCurrent = INPUT_DOUBLE_CLICK;
			break;
		case INPUT_DOUBLE_CLICK:
			InputCurrent = INPUT_TRIPLE_CLICK;
			break;
		}
	}

	// Poping
	if (preButtonState && !ButtonState)
	{
		ButtonTimer.reset();
		ButtonTimer.start();
	}

	// Long press
	if (ButtonState)
	{
		switch (InputCurrent)
		{
		case INPUT_SINGLE_CLICK:
			if (ButtonTimer.read_ms() >= InputLongPressTime) InputCurrent = INPUT_LONG_PRESS;
			break;
		case INPUT_LONG_PRESS:
			if (ButtonTimer.read_ms() >= InputSuperLongPressTime) InputCurrent = INPUT_SUPER_LONG_PRESS;
			break;
		case INPUT_SUPER_LONG_PRESS:
			if (ButtonTimer.read_ms() >= InputUltraLongPressTime) InputCurrent = INPUT_ULTRA_LONG_PRESS;
			break;
		}
	}

	// Detect confirm
	if (!ButtonState && ButtonTimer.read_ms() >= InputWaitForClickTime)
	{
		InputConfirm = InputCurrent;
	}
}

bool InputIsCapturing()
{
	return InputConfirm == INPUT_CAPTURING;
}

INPUT_TYPE InputGetCurrentValue()
{
    return InputCurrent;
}

INPUT_TYPE InputGetConfirmValue()
{
    return InputConfirm;
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
