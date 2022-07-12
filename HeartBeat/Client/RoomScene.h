#pragma once

#include "Scene.h"
#include "../IOCPServer/Protocol.h"

class Mesh;
class Texture;

class RoomScene : public Scene
{
public:
	RoomScene(Client* owner);

	virtual void Enter() override;
	virtual void Exit() override;
	virtual void ProcessInput() override;

private:
	void createUI();
	void createCharacter(int clientID, string_view clientName);
	void createPlayerUI(const int clientID, string_view name);
	void createButtons();
	float getXPosition(int clientID);

	void processNotifyEnterUpgrade(const PACKET& packet);
	void processNotifyLeaveRoom(const PACKET& packet);
	void processNotifyEnterRoom(const PACKET& packet);

	std::tuple<Mesh*, Texture*> getCharacterBelt(int clientID);

	void doWhenChangeToLobbyScene();

private:
	bool mbToUpgrade = false;
	bool mbToRoom = false;
};

