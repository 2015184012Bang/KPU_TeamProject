#pragma once

#include "Scene.h"

class LoginScene : public Scene
{
public:
	LoginScene(Client* owner);

	virtual void Enter() override;
	virtual void Exit() override;
	virtual void ProcessInput() override;

private:
	void processPacket(MemoryStream* packet);
	void processLoginConfirmed(MemoryStream* packet);

private:
	Entity mBackground;

	bool mbConnected;
	bool mbChangeScene;

	TCPSocketPtr mSocket;
};

