#include "ClientPCH.h"
#include "Input.h"

#include "Application.h"

vector<KeyState> Input::sKeyStates;
HWND Input::sHwnd;

void Input::StaticInit()
{
	sHwnd = Application::GetHwnd();
	sKeyStates.resize(256, KeyState::None);
}

void Input::Update()
{
	HWND hwnd = GetActiveWindow();
	if (sHwnd != hwnd)
	{
		for (UINT key = 0; key < KEY_COUNT; key++)
		{
			sKeyStates[key] = KeyState::None;
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

			if (state == KeyState::Repeat || state == KeyState::Press)
				state = KeyState::Repeat;
			else
				state = KeyState::Press;
		}
		else
		{
			KeyState& state = sKeyStates[key];

			if (state == KeyState::Repeat || state == KeyState::Press)
				state = KeyState::Release;
			else
				state = KeyState::None;
		}
	}
}

bool Input::IsButtonRepeat(KeyCode key)
{
	return sKeyStates[static_cast<int>(key)] == KeyState::Repeat;
}

bool Input::IsButtonPressed(KeyCode key)
{
	return sKeyStates[static_cast<int>(key)] == KeyState::Press;
}

bool Input::IsButtonReleased(KeyCode key)
{
	return sKeyStates[static_cast<int>(key)] == KeyState::Release;
}
