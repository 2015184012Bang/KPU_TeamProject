#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>

using std::wstring;
using std::string;

constexpr float EPSILON = 0.001f;

inline wstring s2ws(const string& s)
{
	unsigned int len;
	unsigned int slength = static_cast<unsigned int>(s.length()) + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	wstring ret(buf);
	delete[] buf;
	return ret;
}

inline bool NearZero(const Vector3& a, const Vector3& b)
{
	return (a - b).LengthSquared() < EPSILON;
}

inline bool NearZero(float a)
{
	if (fabs(a) < EPSILON)
	{
		return true;
	}

	return false;
}