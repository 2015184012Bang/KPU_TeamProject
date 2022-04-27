#pragma once

#include <string_view>
using std::string_view;

class IResource
{
public:
	virtual void Load(string_view path) = 0;
};