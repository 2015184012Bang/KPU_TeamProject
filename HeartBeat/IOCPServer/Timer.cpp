#include "pch.h"
#include "Timer.h"

UINT64 Timer::sFrequency;
UINT64 Timer::sPrevCount;
float Timer::sDeltaTime;
priority_queue<TimerEvent> Timer::sTimerEvents;


void Timer::Init()
{
	::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&sFrequency));
	::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&sPrevCount));
}

void Timer::Update()
{
	UINT64 currentCount = 0;
	::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currentCount));

	sDeltaTime = (currentCount - sPrevCount) / static_cast<float>(sFrequency);

	if (sDeltaTime > 0.05f)
	{
		sDeltaTime = 0.05f;
	}

	sPrevCount = currentCount;

	while (!sTimerEvents.empty())
	{
		auto timerEvent = sTimerEvents.top();
		
		if (system_clock::now() < timerEvent.ActTime)
		{
			break;
		}

		sTimerEvents.pop();
		timerEvent.Func();
	}
}

void Timer::Clear()
{
	while (!sTimerEvents.empty())
	{
		sTimerEvents.pop();
	}
}

void Timer::AddEvent(float dueTime, std::function<void()> func)
{
	int sec = static_cast<int>(dueTime);
	int milliSec = static_cast<int>((dueTime - sec) * 1000);

	sTimerEvents.emplace(func, system_clock::now() + static_cast<seconds>(sec)
		+ static_cast<milliseconds>(milliSec));
}

