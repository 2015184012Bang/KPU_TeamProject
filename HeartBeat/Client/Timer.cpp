#include "ClientPCH.h"
#include "Timer.h"

uint64 Timer::sFrequency;
uint64 Timer::sPrevCount;
float Timer::sDeltaTime;
int Timer::sFPS;

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
	sFPS = static_cast<int>(1.0f / sDeltaTime);

	if (sDeltaTime > 0.05f)
	{
		sDeltaTime = 0.05f;
	}


	sPrevCount = currentCount;
}
