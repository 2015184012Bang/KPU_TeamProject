#pragma once

#include <string>

using std::wstring;

class IResource
{
public:
	virtual void Load(const wstring& path) = 0;
};