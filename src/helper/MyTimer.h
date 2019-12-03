#pragma once

class MyTimer
{
private:
	uint64_t startTime;

public:
	MyTimer();

	void start();
	void reset();
	int read_ms();

};