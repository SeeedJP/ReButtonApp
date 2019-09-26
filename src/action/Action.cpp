#include <Arduino.h>
#include "Action.h"
#include "ActionSendMessage.h"
#include "ActionConnectedSendMessage.h"
#include "ActionAccessPoint.h"
#include "ActionFactoryReset.h"

bool ActionTaskBlocking(ACTION_TYPE action)
{
	switch (action)
	{
	case ACTION_1:
	case ACTION_2:
	case ACTION_3:
	case ACTION_10:
	case ACTION_11:
		return ActionSendMessage(action);
	case ACTION_CONNECTED:
		return ActionConnectedSendMessage();
	case ACTION_AP:
		return ActionAccessPoint();
	case ACTION_FACTORY_RESET:
		return ActionFactoryReset();
	default:
		return false;
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
	case ACTION_FACTORY_RESET:
		return "ACTION_FACTORY_RESET";
	default:
		return "UNKNOWN";
	}
}