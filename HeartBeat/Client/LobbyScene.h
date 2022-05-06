#pragma once

#include "Scene.h"
#include "../IOCPServer/Protocol.h"

class Mesh;
class Texture;

class LobbyScene : public Scene
{
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
	void createButtons();
	float getXPosition(int clientID);

	void processNofifyLogin(const PACKET& packet);
	void processNotifyEnterUpgrade(const PACKET& packet);
	void processNotifyLeaveRoom(const PACKET& packet);

	std::tuple<Mesh*, Texture*> getCharacterBelt(int clientID);

	void doWhenChangeToRoomScene();

private:
	bool mbToUpgrade = false;
	bool mbToRoom = false;
};

