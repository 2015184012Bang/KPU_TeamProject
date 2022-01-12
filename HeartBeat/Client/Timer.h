#pragma once

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

