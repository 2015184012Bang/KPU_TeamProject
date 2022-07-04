#pragma once

struct TimerEvent
{
	TimerEvent(function<void()> func, const system_clock::time_point& actTime)
		: Func{ func }
		, ActTime{ actTime }
	{}

	function<void()> Func;
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
	static void AddEvent(float dueTime, std::function<void()> func);

private:
	static UINT64 sFrequency;
	static UINT64 sPrevCount;
	static float sDeltaTime;
	static priority_queue<TimerEvent> sTimerEvents;
};

