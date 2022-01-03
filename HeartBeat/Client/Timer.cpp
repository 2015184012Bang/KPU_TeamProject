#include "ClientPCH.h"
#include "Timer.h"

UINT64 Timer::sFrequency;
UINT64 Timer::sPrevCount;
float Timer::sDeltaTime;

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
}


