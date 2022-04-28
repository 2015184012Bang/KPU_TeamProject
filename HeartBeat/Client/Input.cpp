#include "ClientPCH.h"
#include "Input.h"

#include "Application.h"

vector<KeyState> Input::sKeyStates;
HWND Input::sHwnd;

void Input::Init()
{
	sHwnd = Application::GetHwnd();
	sKeyStates.resize(256, KeyState::NONE);
}

void Input::Update()
{
	HWND hwnd = GetActiveWindow();
	if (sHwnd != hwnd)
	{
		for (UINT key = 0; key < KEY_COUNT; key++)
		{
			sKeyStates[key] = KeyState::NONE;
		}

		return;
	}

	BYTE asciiKeys[KEY_COUNT] = {};
	if (GetKeyboardState(asciiKeys) == false)
	{
		return;
	}

	for (int key = 0; key < KEY_COUNT; key++)
	{
		if (asciiKeys[key] & 0x80)
		{
			KeyState& state = sKeyStates[key];

			if (state == KeyState::REPEAT || state == KeyState::PRESS)
				state = KeyState::REPEAT;
			else
				state = KeyState::PRESS;
		}
		else
		{
			KeyState& state = sKeyStates[key];

			if (state == KeyState::REPEAT || state == KeyState::PRESS)
				state = KeyState::RELEASE;
			else
				state = KeyState::NONE;
		}
	}
}

bool Input::IsButtonRepeat(KeyCode key)
{
	return sKeyStates[static_cast<int>(key)] == KeyState::REPEAT;
}

bool Input::IsButtonPressed(KeyCode key)
{
	return sKeyStates[static_cast<int>(key)] == KeyState::PRESS;
}

bool Input::IsButtonReleased(KeyCode key)
{
	return sKeyStates[static_cast<int>(key)] == KeyState::RELEASE;
}
