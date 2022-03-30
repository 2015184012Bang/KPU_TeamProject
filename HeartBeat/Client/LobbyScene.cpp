#include "ClientPCH.h"
#include "LobbyScene.h"

#include "Client.h"

LobbyScene::LobbyScene(Client* owner)
	: Scene(owner)
{

}

void LobbyScene::Enter()
{
	mSocket = mOwner->GetMySocket();

	HB_LOG("LobbyScene::Enter - Alive Entity: {0}", mOwner->GetRegistry().alive());
}

void LobbyScene::Exit()
{
	HB_LOG("LobbyScene::Exit - Alive Entity: {0}", mOwner->GetRegistry().alive());
}

void LobbyScene::ProcessInput()
{
	MemoryStream packet;
	int retVal = mSocket->Recv(&packet, sizeof(MemoryStream));

	if (retVal == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		if (error != WSAEWOULDBLOCK)
		{
			SocketUtil::ReportError(L"LobbyScene::ProcessInput", error);
			mOwner->SetRunning(false);
		}
	}
	else
	{
		// TODO:: ProcessPacket
	}
}
