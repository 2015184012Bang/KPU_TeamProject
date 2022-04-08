#pragma once

#include <random>

class Random
{
public:
	static void Init();
	static float RandFloat(float min = 0.0f, float max = 1.0f);
	static int RandInt(int min = 0, int max = INT_MAX);

private:
	static std::mt19937 sEngine;
};



