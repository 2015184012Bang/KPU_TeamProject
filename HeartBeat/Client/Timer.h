#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <functional>
#include <queue>

struct TimerEvent
{
	TimerEvent(std::function<void()> func, const system_clock::time_point& actTime)
		: Func{ func }
		, ActTime{ actTime }
	{}

	std::function<void()> Func;
	system_clock::time_point ActTime = {};

	bool operator < (const TimerEvent& other) const
	{
		return ActTime > other.ActTime;
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
	static std::priority_queue<TimerEvent> sTimerEvents;
};
