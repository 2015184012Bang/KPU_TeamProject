#include "ClientPCH.h"
#include "TestScene.h"

#include "Animation.h"
#include "Client.h"
#include "ClientSystems.h"
#include "Input.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Text.h"
#include "Skeleton.h"

#include "CharacterMovement.h"
#include "UIButtonTest.h"
#include "UIButtonTest2.h"

TestScene::TestScene(Client* owner)
	: Scene(owner)
{

}

void TestScene::Enter()
{
	HB_LOG("TestScene::Enter");

	mSocket = mOwner->GetMySocket();
}

void TestScene::Exit()
{
	HB_LOG("TestScene::Exit");
}

void TestScene::ProcessInput()
{
	MemoryStream packet;
	int retVal = mSocket->Recv(&packet, sizeof(MemoryStream));

	if (retVal == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();

		if (errorCode != WSAEWOULDBLOCK)
		{
			SocketUtil::ReportError(L"TestScene::ProcessInput", errorCode);
			mOwner->SetRunning(false);
		}
	}
	else
	{
		processPacket(&packet);
	}
}

void TestScene::Update(float deltaTime)
{

}

void TestScene::processPacket(MemoryStream* packet)
{

}

