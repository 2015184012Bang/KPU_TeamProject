#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

class Timer
{
public:
	static void Init();
	static void Update();

	static float GetDeltaTime() { return sDeltaTime; }

private:
	static uint64 sFrequency;
	static uint64 sPrevCount;
	static float sDeltaTime;
};
