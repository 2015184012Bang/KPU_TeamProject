#pragma once

#include "Script.h"
#include "ClientComponents.h"

class UIButtonTest : public Script
{
public:
	UIButtonTest(Entity owner)
		: Script(owner) {}

	virtual void Start() override
	{
		button = &GetComponent<ButtonComponent>();
		button->CallbackFunc = std::bind(&UIButtonTest::OnClick, this);
	}

	virtual void Update(float deltaTime) override
	{

	}

	virtual void OnClick()
	{
		HB_LOG("Immortan joe looked at me!");
	}

private:
	ButtonComponent* button;
};