#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

struct TimerEvent
{
	TimerEvent(const float dueTime, std::function<void()> func)
		: DueTime(dueTime)
		, Func(func) {}

	float DueTime = 0.0f;
	std::function<void()> Func;

	bool operator<(const TimerEvent& other) const
	{
		return DueTime > other.DueTime;
	}
};

class Timer
{
public:
	static void Init();
	static void Update();
	static void Clear();

	static float GetDeltaTime() { return sDeltaTime; }
	static int GetFPS() { return sFPS; }

	static void AddEvent(float dueTime, std::function<void()> func);

private:
	static uint64 sFrequency;
	static uint64 sPrevCount;
	static float sDeltaTime;
	static int sFPS;
	static std::vector<TimerEvent> sTimerEvents;
};
