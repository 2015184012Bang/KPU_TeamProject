#pragma once

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

