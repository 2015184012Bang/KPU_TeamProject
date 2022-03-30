#pragma once

#include "Scene.h"

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

private:
	TCPSocketPtr mSocket;

	vector<int> mConnectedID;

	Entity mReadyButton;
};

