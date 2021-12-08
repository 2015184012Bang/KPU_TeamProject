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
	static void StaticInit();
	static void Update();

	static bool IsButtonRepeat(KeyCode key);
	static bool IsButtonPressed(KeyCode key);
	static bool IsButtonReleased(KeyCode key);

private:
	static const int KEY_COUNT = 256;

	static vector<KeyState> sKeyStates;

	static HWND sHwnd;
};

