#pragma once

enum ACTION_TYPE
{
	ACTION_NONE,
	ACTION_1,
	ACTION_2,
	ACTION_3,
	ACTION_10,
	ACTION_11,
	ACTION_AP,
	ACTION_FACTORY_RESET,
};

bool ActionTaskBlocking(ACTION_TYPE action);
const char* ActionGetActionString(ACTION_TYPE value);
