#pragma once

#pragma region Component_Test
struct HelloComponent
{
	HelloComponent()
		: Phrase(L"Hello, world!") {}

	HelloComponent(const wstring& p)
		: Phrase(p) {}

	wstring Phrase;
};

struct TransformComponent
{
	TransformComponent()
		: X(0.0f) {}

	TransformComponent(float x)
		: X(x) {}

	float X;
};
#pragma endregion