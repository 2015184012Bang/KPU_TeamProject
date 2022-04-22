#pragma once

#include "Scene.h"


class LobbyScene : public Scene
{
	const float SPACE_BETWEEN_LINES = 30.0f;
	const float WIDTH_BETWEEN_CHARACTERS = 700.0f;

public:
	LobbyScene(Client* owner);

	virtual void Enter() override;
	virtual void Exit() override;
	virtual void ProcessInput() override;

private:
	void processPacket(MemoryStream* packet);
	void processUserConnected(MemoryStream* packet);
	void processReadyPressed(MemoryStream* packet);
	void processGameStart(MemoryStream* packet);
	
	void createNicknameText(int clientID);
	void createCharacterMesh(int clientID);

private:
	vector<int> mConnectedID;
	Entity mReadyText;
};

