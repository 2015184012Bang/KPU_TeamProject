#pragma once

#include "Scene.h"
#include "../IOCPServer/Protocol.h"


class LobbyScene : public Scene
{
	const int HOST_ID = 2;
	const float WIDTH_BETWEEN_CHARACTERS = 700.0f;
	const int START_BUTTON_WIDTH = 200;
	const int START_BUTTON_HEIGHT = 100;
	const float START_BUTTON_DIST_FROM_BOTTOM = 150.0f;

public:
	LobbyScene(Client* owner);

	virtual void Enter() override;
	virtual void Exit() override;
	virtual void ProcessInput() override;

private:
	void createCharacterMesh(int clientID);
	float getXPosition(int clientID);

	void processAnswerNofifyLogin(const PACKET& packet);
	void processAnswerGameStart(const PACKET& packet);

private:
	bool mbChangeScene = false;
};

