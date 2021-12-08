#pragma once

class Timer
{
public:
	static void StaticInit();
	static void Update();

	static float GetDeltaTime() { return sDeltaTime; }

private:
	static UINT64 sFrequency;
	static UINT64 sPrevCount;
	static float sDeltaTime;
};

