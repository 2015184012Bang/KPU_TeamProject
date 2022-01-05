#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

enum class eKeyCode
{
	Up = VK_UP,
	Down = VK_DOWN,
	Left = VK_LEFT,
	Right = VK_RIGHT,
	Return = VK_RETURN,
	Escape = VK_ESCAPE,

	W = 'W',
	A = 'A',
	S = 'S',
	D = 'D',

	MouseLButton = VK_LBUTTON,
	MouseRButton = VK_RBUTTON,
};