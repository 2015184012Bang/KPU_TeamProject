#pragma once

struct TimerEvent
{
	TimerEvent(const float dueTime, std::function<void()> func)
		: DueTime(dueTime)
		, Func(func) {}

	float DueTime = 0.0f;
	std::function<void()> Func;
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
	static vector<TimerEvent> sTimerEvents;
};

