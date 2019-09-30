#include <Arduino.h>
#include "../Common.h"
#include "Input.h"

#include <ReButton.h>

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
			if (ButtonTimer.read_ms() >= INPUT_LONG_PRESS_TIME) InputCurrent = INPUT_LONG_PRESS;
			break;
		case INPUT_LONG_PRESS:
			if (ButtonTimer.read_ms() >= INPUT_SUPER_LONG_PRESS_TIME) InputCurrent = INPUT_SUPER_LONG_PRESS;
			break;
		case INPUT_SUPER_LONG_PRESS:
			if (ButtonTimer.read_ms() >= INPUT_ULTRA_LONG_PRESS_TIME) InputCurrent = INPUT_ULTRA_LONG_PRESS;
			break;
		}
	}

	// Detect confirm
	if (!ButtonState && ButtonTimer.read_ms() >= INPUT_WAIT_FOR_CLICK_TIME)
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
