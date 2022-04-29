#include "pch.h"
#include "Random.h"

std::mt19937 Random::sEngine;

void Random::Init()
{
	std::random_device rd;
	sEngine.seed(rd());
}

float Random::RandFloat(float min, float max)
{
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(sEngine);
}

int Random::RandInt(int min /*= 0*/, int max /*= INT_MAX*/)
{
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(sEngine);
}