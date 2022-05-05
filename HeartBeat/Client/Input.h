#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

enum class KeyCode
{
	UP = VK_UP,
	DOWN = VK_DOWN,
	LEFT = VK_LEFT,
	RIGHT = VK_RIGHT,
	RETURN = VK_RETURN,
	ESCAPE = VK_ESCAPE,
	SPACE = VK_SPACE,

	W = 'W',
	A = 'A',
	S = 'S',
	D = 'D',

	MOUSE_L = VK_LBUTTON,
	MOUSE_R = VK_RBUTTON,
};

enum class KeyState
{
	NONE,
	REPEAT,
	PRESS,
	RELEASE
};

class Input
{
public:
	static void Init();
	static void Update();

	static bool IsButtonRepeat(KeyCode key);
	static bool IsButtonPressed(KeyCode key);
	static bool IsButtonReleased(KeyCode key);

private:
	static const int KEY_COUNT = 256;

	static vector<KeyState> sKeyStates;

	static HWND sHwnd;
};

