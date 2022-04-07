#pragma once

#include "Scene.h"

constexpr float SPACE_BETWEEN_LINES = 30.0f;
constexpr float WIDTH_BETWEEN_CHARACTERS = 700.0f;




class LobbyScene : public Scene
{
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
	TCPSocketPtr mSocket;

	vector<int> mConnectedID;

	Entity mReadyText;
};

