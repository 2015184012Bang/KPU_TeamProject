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
	Space = VK_SPACE,

	W = 'W',
	A = 'A',
	S = 'S',
	D = 'D',

	MouseLButton = VK_LBUTTON,
	MouseRButton = VK_RBUTTON,
};

enum class KeyState
{
	None,
	Repeat,
	Press,
	Release
};

class Input
{
public:
	static void Init();
	static void Update();

	static bool IsButtonRepeat(eKeyCode key);
	static bool IsButtonPressed(eKeyCode key);
	static bool IsButtonReleased(eKeyCode key);

private:
	static const int KEY_COUNT = 256;

	static vector<KeyState> sKeyStates;

	static HWND sHwnd;
};

