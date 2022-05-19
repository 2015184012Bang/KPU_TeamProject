#include "pch.h"
#include "Timer.h"

UINT64 Timer::sFrequency;
UINT64 Timer::sPrevCount;
float Timer::sDeltaTime;
vector<TimerEvent> Timer::sTimerEvents;


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

	auto iter = sTimerEvents.begin();
	while (iter != sTimerEvents.end())
	{
		iter->DueTime -= sDeltaTime;

		if (iter->DueTime < 0.0f)
		{
			iter->Func();
			iter = sTimerEvents.erase(iter);
		}
		else
		{
			++iter;
		}
	}

	sPrevCount = currentCount;
}

void Timer::AddEvent(float dueTime, std::function<void()> func)
{
	sTimerEvents.emplace_back(dueTime, func);
}

