#pragma once

#include "Scene.h"

class LobbyScene : public Scene
{
public:
	LobbyScene(Client* owner);

	virtual void Enter() override;
	virtual void Exit() override;

private:
	
};

