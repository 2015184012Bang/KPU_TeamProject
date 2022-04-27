#pragma once

#include <string>

class IResource
{
public:
	virtual void Load(const string& path) = 0;
};