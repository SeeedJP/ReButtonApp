#pragma once

void AutoShutdownBegin(int timeout);
void AutoShutdownUpdateStartTime();
void AutoShutdownSetTimeout(int timeout);
void AutoShutdownSuspend();
void AutoShutdownResume();
