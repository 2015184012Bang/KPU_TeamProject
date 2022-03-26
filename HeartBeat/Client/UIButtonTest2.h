#pragma once

#include "Script.h"
#include "ClientComponents.h"

class UIButtonTest2 : public Script
{
public:
	UIButtonTest2(Entity owner)
		: Script(owner) {}

	virtual void Start() override
	{
		button = &GetComponent<ButtonComponent>();
		button->CallbackFunc = std::bind(&UIButtonTest2::OnClick, this);
	}

	virtual void Update(float deltaTime) override
	{

	}

	virtual void OnClick()
	{
		HB_LOG("The god myung kyu looked at me!");
	}

private:
	ButtonComponent* button;
};