#include "ClientPCH.h"
#include "GameScene.h"

#include "Animation.h"
#include "Client.h"
#include "ClientSystems.h"
#include "Input.h"
#include "HeartBeat/PacketType.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Text.h"
#include "Skeleton.h"

#include "Character.h"
#include "EnemyMovement.h"

GameScene::GameScene(Client* owner)
	: Scene(owner)
{

}

void GameScene::Enter()
{
	HB_LOG("TestScene::Enter");

	mSocket = mOwner->GetMySocket();

	Entity e = mOwner->CreateStaticMeshEntity(L"Assets/Meshes/Cube.mesh", L"Assets/Textures/Cube_Pink.png");
	auto& trasnform = e.GetComponent<TransformComponent>();

	trasnform.Position.y = -500.0f;
}

void GameScene::Exit()
{
	HB_LOG("TestScene::Exit");
}

void GameScene::ProcessInput()
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

void GameScene::Update(float deltaTime)
{
	Vector3 direction = Vector3::Zero;
	bool bMove = false;

	if (Input::IsButtonRepeat(eKeyCode::Up))
	{
		direction.z += 1.0f;
		bMove = true;
	}

	if (Input::IsButtonRepeat(eKeyCode::Down))
	{
		direction.z -= 1.0f;
		bMove = true;
	}

	if (Input::IsButtonRepeat(eKeyCode::Left))
	{
		direction.x -= 1.0f;
		bMove = true;
	}

	if (Input::IsButtonRepeat(eKeyCode::Right))
	{
		direction.x += 1.0f;
		bMove = true;
	}

	if (bMove)
	{
		MemoryStream packet;
		packet.WriteUByte(static_cast<uint8>(CSPacket::eUserInput));
		packet.WriteUInt64(mMyEID);
		packet.WriteVector3(direction);

		mSocket->Send(&packet, sizeof(packet));
	}
}

void GameScene::processPacket(MemoryStream* packet)
{
	int totalLen = packet->GetLength();
	packet->SetLength(0);

	while (packet->GetLength() < totalLen)
	{
		uint8 packetType;
		packet->ReadUByte(&packetType);

		switch (static_cast<SCPacket>(packetType))
		{
		case SCPacket::eCreateCharacter:
			processCreateCharacter(packet);
			break;

		case SCPacket::eUpdateTransform:
			processUpdateTransform(packet);
			break;

		default:
			HB_LOG("Unknown packet type: {0}", static_cast<int>(packetType));
			packet->SetLength(totalLen);
			break;
		}
	}
}

void GameScene::processCreateCharacter(MemoryStream* packet)
{
	for (int i = 0; i < 3; ++i)
	{
		int clientID = -1;
		uint64 entityID = 0;

		packet->ReadInt(&clientID);
		packet->ReadUInt64(&entityID);

		wstring meshFile;
		wstring texFile;
		wstring skelFile;

		GetCharacterFiles(clientID, &meshFile, &texFile, &skelFile);

		Entity e = mOwner->CreateSkeletalMeshEntity(meshFile, texFile, skelFile, entityID);
		e.AddTag<Tag_Player>();
		auto& animator = e.GetComponent<AnimatorComponent>();

		wstring idleAnimFile = GetCharacterAnimation(clientID, CharacterAnimationType::eIdle);
		ClientSystems::PlayAnimation(&animator, ResourceManager::GetAnimation(idleAnimFile));

		// 클라이언트 ID가 나라면, 스크립트를 부착하고 따로 저장해둔다
		if (clientID == mOwner->GetClientID())
		{
			mMyCharacter = e;
			mMyCharacter.AddComponent<ScriptComponent>(new Character(mMyCharacter));
			mMyEID = entityID;
		}
	}
}

void GameScene::processUpdateTransform(MemoryStream* packet)
{
	uint64 eid;
	packet->ReadUInt64(&eid);
	Vector3 position;
	packet->ReadVector3(&position);

	float rotationY;
	packet->ReadFloat(&rotationY);

	auto e = mOwner->GetEntityByID(eid);
	if (entt::null == e)
	{
		HB_ASSERT(false, "Unknown ID: {0}", eid);
	}

	Entity ent = Entity(e, mOwner);

	auto& transform = ent.GetComponent<TransformComponent>();
	
	ClientSystems::UpdatePosition(&transform.Position, position, &transform.bDirty);
	ClientSystems::UpdateYRotation(&transform.Rotation.y, rotationY, &transform.bDirty);
}

