#include <Arduino.h>
#include "../Common.h"
#include "Display.h"

#include <ReButton.h>

enum DISPLAY_STATE_TYPE
{
	DISPLAY_STATE_LIGHTING,
	DISPLAY_STATE_ACTION,
	DISPLAY_STATE_FINISH,
	DISPLAY_STATE_ACTION_DISCONNECTED,
	DISPLAY_STATE_ACTION_CONNECTED,
};

struct DISPLAY_MESSAGE
{
	DISPLAY_STATE_TYPE State;
	DISPLAY_COLOR_TYPE Color;
};

static Thread DisplayThread(osPriorityAboveNormal);
static Mail<DISPLAY_MESSAGE, 16> DisplayMailbox;

static void DisplayMain()
{
	DISPLAY_MESSAGE lastMessage;
	lastMessage.State = DISPLAY_STATE_LIGHTING;
	lastMessage.Color.Red = 0;
	lastMessage.Color.Green = 0;
	lastMessage.Color.Blue = 0;
	Timer lastMessageTimer;

	for (;;)
	{
		osEvent evt = DisplayMailbox.get(50);
		if (evt.status == osEventMail)
		{
			DISPLAY_MESSAGE* message = (DISPLAY_MESSAGE*)evt.value.p;
			lastMessage = *message;
			DisplayMailbox.free(message);

			switch (lastMessage.State)
			{
			case DISPLAY_STATE_LIGHTING:
				ReButton::SetLed((float)lastMessage.Color.Red / 255.0f, (float)lastMessage.Color.Green / 255.0f, (float)lastMessage.Color.Blue / 255.0f);
				break;
			case DISPLAY_STATE_ACTION:
			case DISPLAY_STATE_FINISH:
			case DISPLAY_STATE_ACTION_DISCONNECTED:
			case DISPLAY_STATE_ACTION_CONNECTED:
				lastMessageTimer.reset();
				lastMessageTimer.start();
				break;
			}
		}

		int elapsedTime;
		switch (lastMessage.State)
		{
		case DISPLAY_STATE_ACTION:
			elapsedTime = lastMessageTimer.read_ms();
			if (elapsedTime < 200) ReButton::SetLed(0.0f, 0.0f, 0.0f);
			else if (elapsedTime < 400) ReButton::SetLed((float)lastMessage.Color.Red / 255.0f, (float)lastMessage.Color.Green / 255.0f, (float)lastMessage.Color.Blue / 255.0f);
			else if (elapsedTime < 600) ReButton::SetLed(0.0f, 0.0f, 0.0f);
			else if (elapsedTime < 800) ReButton::SetLed((float)lastMessage.Color.Red / 255.0f, (float)lastMessage.Color.Green / 255.0f, (float)lastMessage.Color.Blue / 255.0f);
			else if (elapsedTime < 1000) ReButton::SetLed(0.0f, 0.0f, 0.0f);
			else if (elapsedTime < 1200) ReButton::SetLed((float)lastMessage.Color.Red / 255.0f, (float)lastMessage.Color.Green / 255.0f, (float)lastMessage.Color.Blue / 255.0f);
			else
			{
				elapsedTime -= 1200;
				elapsedTime %= 1000;
				if (elapsedTime < 800)
				{
					ReButton::SetLed(0.0f, 0.0f, 0.0f);
				}
				else
				{
					ReButton::SetLed((float)lastMessage.Color.Red / 255.0f, (float)lastMessage.Color.Green / 255.0f, (float)lastMessage.Color.Blue / 255.0f);
				}
			}
			break;
		case DISPLAY_STATE_FINISH:
			elapsedTime = lastMessageTimer.read_ms();
			if (elapsedTime < 500) ReButton::SetLed(0.0f, 0.0f, 0.0f);
			else if (elapsedTime < 1500) ReButton::SetLed((float)lastMessage.Color.Red / 255.0f, (float)lastMessage.Color.Green / 255.0f, (float)lastMessage.Color.Blue / 255.0f);
			break;
		case DISPLAY_STATE_ACTION_DISCONNECTED:
			elapsedTime = lastMessageTimer.read_ms();
			elapsedTime %= 1000;
			if (elapsedTime < 800)
			{
				ReButton::SetLed(0.0f, 0.0f, 0.0f);
			}
			else
			{
				ReButton::SetLed((float)lastMessage.Color.Red / 255.0f, (float)lastMessage.Color.Green / 255.0f, (float)lastMessage.Color.Blue / 255.0f);
			}
			break;
		case DISPLAY_STATE_ACTION_CONNECTED:
			elapsedTime = lastMessageTimer.read_ms();
			elapsedTime %= 5000;
			if (elapsedTime < 4800)
			{
				ReButton::SetLed(0.0f, 0.0f, 0.0f);
			}
			else
			{
				ReButton::SetLed((float)lastMessage.Color.Red / 255.0f, (float)lastMessage.Color.Green / 255.0f, (float)lastMessage.Color.Blue / 255.0f);
			}
			break;
		}
	}
}

void DisplayBegin()
{
	DisplayThread.start(DisplayMain);
}

void DisplayColor(DISPLAY_COLOR_TYPE color)
{
	DISPLAY_MESSAGE* message = DisplayMailbox.alloc();
	message->State = DISPLAY_STATE_LIGHTING;
	message->Color = color;
	DisplayMailbox.put(message);
}

void DisplayStartAction(DISPLAY_COLOR_TYPE color)
{
	DISPLAY_MESSAGE* message = DisplayMailbox.alloc();
	message->State = DISPLAY_STATE_ACTION;
	message->Color = color;
	DisplayMailbox.put(message);
}

void DisplayStartFinish(DISPLAY_COLOR_TYPE color)
{
	DISPLAY_MESSAGE* message = DisplayMailbox.alloc();
	message->State = DISPLAY_STATE_FINISH;
	message->Color = color;
	DisplayMailbox.put(message);
}

void DisplayStartActionDisconnected(DISPLAY_COLOR_TYPE color)
{
	DISPLAY_MESSAGE* message = DisplayMailbox.alloc();
	message->State = DISPLAY_STATE_ACTION_DISCONNECTED;
	message->Color = color;
	DisplayMailbox.put(message);
}

void DisplayStartActionConnected(DISPLAY_COLOR_TYPE color)
{
	DISPLAY_MESSAGE* message = DisplayMailbox.alloc();
	message->State = DISPLAY_STATE_ACTION_CONNECTED;
	message->Color = color;
	DisplayMailbox.put(message);
}
