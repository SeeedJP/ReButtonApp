#pragma once

struct DISPLAY_COLOR_TYPE
{
	uint8_t Red;
	uint8_t Green;
	uint8_t Blue;
};

extern const DISPLAY_COLOR_TYPE DISPLAY_OFF;
extern const DISPLAY_COLOR_TYPE DISPLAY_OK;
extern const DISPLAY_COLOR_TYPE DISPLAY_ERROR;

void DisplayBegin();
void DisplayColor(DISPLAY_COLOR_TYPE color);
void DisplayStartAction(DISPLAY_COLOR_TYPE color);
void DisplayStartFinish(DISPLAY_COLOR_TYPE color);
