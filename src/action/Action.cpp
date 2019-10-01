#include <Arduino.h>
#include "../Common.h"
#include "Action.h"

#include "ActionSendMessage.h"
#include "ActionConnectedSendMessage.h"
#include "ActionAccessPoint.h"

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
	default:
		return false;
	}
}
