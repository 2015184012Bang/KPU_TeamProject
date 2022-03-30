#include "ClientPCH.h"
#include "LobbyScene.h"

#include "Client.h"

LobbyScene::LobbyScene(Client* owner)
	: Scene(owner)
{

}

void LobbyScene::Enter()
{
	HB_LOG("LobbyScene::Enter - Alive Entity: {0}", mOwner->GetRegistry().alive());
}

void LobbyScene::Exit()
{
	HB_LOG("LobbyScene::Exit - Alive Entity: {0}", mOwner->GetRegistry().alive());
}
