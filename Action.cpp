#include <Arduino.h>
#include "Action.h"
#include "ActionSendMessage.h"
#include "src/action/ActionSendMessagePnP.h"
#include "ActionAccessPoint.h"

bool ActionTaskBlocking(ACTION_TYPE action)
{
	switch (action)
	{
	case ACTION_NONE:
		return true;
	case ACTION_1:
	case ACTION_2:
	case ACTION_3:
	case ACTION_10:
	case ACTION_11:
		return ActionSendMessagePnP(action);
	case ACTION_AP:
		return ActionAccessPoint();
	default:
		return false;
	}
}

const char* ActionGetActionString(ACTION_TYPE value)
{
	switch (value)
	{
	case ACTION_NONE:
		return "ACTION_NONE";
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
	case ACTION_AP:
		return "ACTION_AP";
	default:
		return "UNKNOWN";
	}
}