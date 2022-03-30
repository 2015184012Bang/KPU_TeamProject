#include "ClientPCH.h"
#include "LoginScene.h"

#include "Application.h"
#include "Client.h"
#include "Input.h"
#include "LobbyScene.h"

LoginScene::LoginScene(Client* owner)
	: Scene(owner)
	, mbConnected(false)
	, mbChangeScene(false)
	, mSocket(nullptr)
{

}

void LoginScene::Enter()
{
	mSocket = mOwner->GetMySocket();

	mBackground = mOwner->CreateSpriteEntity(Application::GetScreenWidth(), Application::GetScreenHeight(),
		L"Assets/Textures/Login_Background.png", 10);

	HB_LOG("LoginScene::Enter - Alive Entity: {0}", mOwner->GetRegistry().alive());
}

void LoginScene::Exit()
{
	mOwner->DestroyEntity(mBackground);

	HB_LOG("LoginScene::Exit - Alive Entity: {0}", mOwner->GetRegistry().alive());
}

void LoginScene::ProcessInput()
{
	if (Input::IsButtonPressed(eKeyCode::Return) && !mbConnected)
	{
		SocketAddress serveraddr("127.0.0.1", SERVER_PORT);

		int retVal = mSocket->Connect(serveraddr);

		if (retVal == SOCKET_ERROR)
		{
			SocketUtil::ReportError(L"LoginScene::ProcessInput()");
		}
		else
		{
			mbConnected = true;
			mSocket->SetNonBlockingMode(true);

			// Send LoginRequest packet after connect
			MemoryStream packet;

			string id = "derisan";
			packet.WriteInt(static_cast<int>(CSPacket::eLoginRequest));
			packet.WriteInt(id.size());
			packet.WriteString(id);

			mSocket->Send(&packet, sizeof(MemoryStream));
			
		}
	}

	if (mbConnected)
	{
		MemoryStream packet;

		int retVal = mSocket->Recv(&packet, sizeof(MemoryStream));

		if (retVal == SOCKET_ERROR)
		{
			int error = WSAGetLastError();

			if (error == WSAEWOULDBLOCK)
			{
				return;
			}
			else
			{
				SocketUtil::ReportError(L"LoginScene::ProcessInput", error);
				mOwner->SetRunning(false);
			}
		}
		else if(retVal == 0)
		{
			HB_LOG("Server Disconnected");
			mOwner->SetRunning(false);
		}
		else
		{
			processPacket(&packet);
		}
	}

	if (mbChangeScene)
	{
		mOwner->ChangeScene(new LobbyScene(mOwner));
	}
}

void LoginScene::processPacket(MemoryStream* packet)
{
	int totalLen = packet->GetLength();
	packet->SetLength(0);

	while (packet->GetLength() < totalLen)
	{
		SCPacket packetType;
		packet->ReadInt(reinterpret_cast<int*>(&packetType));

		switch (packetType)
		{
		case SCPacket::eLoginConfirmed:
			processLoginConfirmed(packet);
			break;

		default:
			HB_LOG("Unknown packet type: {0}", static_cast<int>(packetType));
			break;
		}
	}
}

void LoginScene::processLoginConfirmed(MemoryStream* packet)
{
	int myClientID = -1;
	packet->ReadInt(&myClientID);
	mOwner->SetClientID(myClientID);
	mbChangeScene = true;
	HB_LOG("My Client ID: {0}", myClientID);
}
