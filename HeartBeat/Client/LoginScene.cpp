#include "ClientPCH.h"
#include "LoginScene.h"

#include "HeartBeat/PacketType.h"

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
}

void LoginScene::Exit()
{
	mOwner->DestroyEntity(mBackground);
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
			packet.WriteUByte(static_cast<int>(CSPacket::eLoginRequest));
			packet.WriteInt(static_cast<int>(id.size()));
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
		uint8 packetType;
		packet->ReadUByte(&packetType);

		switch (static_cast<SCPacket>(packetType))
		{
		case SCPacket::eLoginConfirmed:
			processLoginConfirmed(packet);
			break;

		default:
			HB_LOG("Unknown packet type: {0}", static_cast<int>(packetType));
			packet->SetLength(totalLen);
			break;
		}
	}
}

void LoginScene::processLoginConfirmed(MemoryStream* packet)
{
	int myClientID = -1;
	packet->ReadInt(&myClientID);
	mOwner->SetClientID(myClientID);

	int nickLen = 0;
	packet->ReadInt(&nickLen);

	string nickname;
	packet->ReadString(&nickname, nickLen);
	mOwner->SetNickname(nickname);

	HB_LOG("Client ID[{0}] // Nickname[{1}]", myClientID, nickname);
	mbChangeScene = true;
}
