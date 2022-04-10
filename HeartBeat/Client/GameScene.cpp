#include "ClientPCH.h"
#include "GameScene.h"

#include "HeartBeat/PacketType.h"
#include "HeartBeat/Tags.h"
#include "HeartBeat/Define.h"
#include "HeartBeat/GameMap.h"

#include "Animation.h"
#include "Client.h"
#include "ClientSystems.h"
#include "Character.h"
#include "Enemy.h"
#include "Input.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Text.h"
#include "Skeleton.h"

GameScene::GameScene(Client* owner)
	: Scene(owner)
{

}

void GameScene::Enter()
{
	HB_LOG("TestScene::Enter");

	mSocket = mOwner->GetMySocket();

	// Map1 생성
	const vector<Tile>& gameMap = gGameMap.GetTiles();
	for (const Tile& tile : gameMap)
	{
		Entity t = mOwner->CreateStaticMeshEntity(MESH(L"Cube.mesh"), GetTileTex(tile.Type));
		t.AddTag<Tag_Tile>();
		auto& transform = t.GetComponent<TransformComponent>();
		transform.Position.x = tile.X;
		transform.Position.y = -TILE_WIDTH;
		transform.Position.z = tile.Z;
	}
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
	sendUserInput();
	updateAnimTrigger();
	updateMainCamera();
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

		case SCPacket::eCreateEnemy:
			processCreateEnemy(packet);
			break;

		case SCPacket::eDeleteEntity:
			processDeleteEntity(packet);
			break;

		case SCPacket::eCreateTank:
			processCreateTank(packet);
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
	for (int i = 0; i < MAX_PLAYER; ++i)
	{
		int clientID = -1;
		uint64 entityID = 0;

		packet->ReadInt(&clientID);
		packet->ReadUInt64(&entityID);

		wstring meshFile;
		wstring texFile;
		wstring skelFile;

		GetCharacterFiles(clientID, &meshFile, &texFile, &skelFile);

		Entity e = mOwner->CreateSkeletalMeshEntity(meshFile, texFile, skelFile, entityID, BOX(L"Character.box"));
		e.AddTag<Tag_Player>();
		auto& animator = e.GetComponent<AnimatorComponent>();

		wstring idleAnimFile = GetCharacterAnimation(clientID, CharacterAnimationType::eIdle);
		ClientSystems::PlayAnimation(&animator, ResourceManager::GetAnimation(idleAnimFile));

		// 클라이언트 ID가 나라면, 스크립트를 부착하고 따로 저장해둔다
		if (clientID == mOwner->GetClientID())
		{
			mMyCharacter = e;
			mMyCharacter.AddComponent<ScriptComponent>(new Character(mMyCharacter));
			mMyCharacterID = entityID;
		}
	}
}

void GameScene::processUpdateTransform(MemoryStream* packet)
{
	uint64 eid;
	packet->ReadUInt64(&eid);
	Vector3 position;
	packet->ReadVector3(&position);

	float yaw;
	packet->ReadFloat(&yaw);

	auto e = mOwner->GetEntityByID(eid);
	if (entt::null == e)
	{
		HB_ASSERT(false, "Unknown ID: {0}", eid);
	}

	Entity ent = Entity(e, mOwner);

	auto& transform = ent.GetComponent<TransformComponent>();
	
	ClientSystems::UpdatePosition(&transform.Position, position, &transform.bDirty);
	ClientSystems::UpdateYRotation(&transform.Rotation.y, yaw, &transform.bDirty);

	auto& animator = ent.GetComponent<AnimatorComponent>();
	animator.SetTrigger("Run");
	
	ent.AddTag<Tag_Moved>();
}

void GameScene::sendUserInput()
{
	if (!mMyCharacter)
	{
		return;
	}

	Vector3 direction = Vector3::Zero;
	bool bMove = false;
	bool bClicked = false;

	if (Input::IsButtonRepeat(eKeyCode::W))
	{
		direction.z += 1.0f;
		bMove = true;
	}

	if (Input::IsButtonRepeat(eKeyCode::S))
	{
		direction.z -= 1.0f;
		bMove = true;
	}

	if (Input::IsButtonRepeat(eKeyCode::A))
	{
		direction.x -= 1.0f;
		bMove = true;
	}

	if (Input::IsButtonRepeat(eKeyCode::D))
	{
		direction.x += 1.0f;
		bMove = true;
	}

	if (Input::IsButtonPressed(eKeyCode::MouseLButton))
	{
		bClicked = true;
	}

	MemoryStream packet;
	if (bMove)
	{
		packet.WriteUByte(static_cast<uint8>(CSPacket::eUserKeyboardInput));
		packet.WriteUInt64(mMyCharacterID);
		packet.WriteVector3(direction);
	}

	if (bClicked)
	{
		packet.WriteUByte(static_cast<uint8>(CSPacket::eUserMouseInput));
		packet.WriteUInt64(mMyCharacterID);
	}

	if (packet.GetLength() > 0)
	{
		mSocket->Send(&packet, sizeof(packet));
	}
}

void GameScene::updateAnimTrigger()
{
	// Moved 태그가 붙은 엔티티는 서버로부터 위치 갱신을 받은 것들이다.
	// Moved 태그가 붙지 않았다면 움직이지 않았다는 것이므로 애니메이션을 Idle로 바꾼다.
	{
		auto view = mOwner->GetRegistry().view<Tag_Player>();
		for (auto entity : view)
		{
			Entity e = Entity(entity, mOwner);

			if (!e.HasComponent<Tag_Moved>())
			{
				auto& animator = e.GetComponent<AnimatorComponent>();
				animator.SetTrigger("Idle");
			}
			else
			{
				e.RemoveComponent<Tag_Moved>();
			}
		}
	}

	{
		auto view = mOwner->GetRegistry().view<Tag_Enemy>();
		for (auto entity : view)
		{
			Entity e = Entity(entity, mOwner);

			if (!e.HasComponent<Tag_Moved>())
			{
				auto& animator = e.GetComponent<AnimatorComponent>();
				animator.SetTrigger("Idle");
			}
			else
			{
				e.RemoveComponent<Tag_Moved>();
			}
		}
	}
}

void GameScene::updateChildParentAfterDelete()
{
	// HACK: 엔티티 삭제 시 AttachmentChild 컴포넌트의 메모리가 오염됨.
	// entt 내부의 문제로 생각되며 Attachment 컴포넌트를 가진 엔티티들을
	// 갱신하는 것으로 해결.
	auto view = mOwner->GetRegistry().view<AttachmentParentComponent, TransformComponent, AnimatorComponent>();

	for (auto [entity, parent, transform, animator] : view.each())
	{
		Entity child( mOwner->GetEntityByID(parent.ChildID), mOwner );
		auto& attachmentChild = child.GetComponent<AttachmentChildComponent>();

		attachmentChild.ParentPalette = &animator.Palette;
		attachmentChild.BoneIndex = animator.Skel->GetBoneIndexByName("Weapon");
		attachmentChild.ParentTransform = &transform;
	}
}

void GameScene::updateMainCamera()
{
	if (!mMyCharacter)
	{
		return;
	}

	Entity& mainCamera = mOwner->GetMainCamera();

	auto& camera = mainCamera.GetComponent<CameraComponent>();
	
	auto& characterTransform = mMyCharacter.GetComponent<TransformComponent>();
	auto& characterPosition = characterTransform.Position;

	camera.Target = characterPosition;
	camera.Position = Vector3(characterPosition.x, 1000.0f, characterPosition.z - 1000.0f);
}

void GameScene::processCreateTank(MemoryStream* packet)
{
	uint64 eid;
	packet->ReadUInt64(&eid);
	
	Vector3 position = Vector3::Zero;
	packet->ReadVector3(&position);

	float yaw = 0.0f;
	packet->ReadFloat(&yaw);

	Entity tank = mOwner->CreateSkeletalMeshEntity(MESH(L"Tank.mesh"), TEXTURE(L"Tank.png"), SKELETON(L"Tank.skel"), eid);
	tank.AddTag<Tag_Tank>();
	auto& transform = tank.GetComponent<TransformComponent>();

	transform.Position = position;
	transform.Rotation.y = yaw;

	auto& animator = tank.GetComponent<AnimatorComponent>();
	ClientSystems::PlayAnimation(&animator, ResourceManager::GetAnimation(ANIM(L"Tank_Idle.anim")));
}

void GameScene::processDeleteEntity(MemoryStream* packet)
{
	uint64 eid;
	packet->ReadUInt64(&eid);

	Entity e{ mOwner->GetEntityByID(eid), mOwner };
	bool hasChild = false;
	if (e.HasComponent<AttachmentParentComponent>())
	{
		hasChild = true;
	}

	mOwner->DestroyEntityByID(eid);

	// Attachement 컴포넌트가 있다면 갱신한다.
	if (hasChild)
	{
		updateChildParentAfterDelete();
	}
}

void GameScene::processCreateEnemy(MemoryStream* packet)
{
	uint64 eid;
	packet->ReadUInt64(&eid);

	uint8 enemyType;
	packet->ReadUByte(&enemyType);

	Vector3 position;
	packet->ReadVector3(&position);

	wstring meshFile;
	wstring texFile;
	wstring skelFile;

	GetEnemyFiles(enemyType, &meshFile, &texFile, &skelFile);
	wstring idleAnimFile = GetEnemyAnimation(enemyType, EnemyAnimationType::eIdle);

	Entity enemy = mOwner->CreateSkeletalMeshEntity(meshFile, texFile, skelFile, eid, BOX(L"Virus.box"));
	enemy.AddTag<Tag_Enemy>();

	auto& transform = enemy.GetComponent<TransformComponent>();
	transform.Position = position;

	if (enemyType == Virus)
	{
		Entity pickax = mOwner->CreateStaticMeshEntity(MESH(L"Pickax.mesh"), TEXTURE(L"Pickax.png"));
		ClientSystems::SetBoneAttachment(enemy, pickax, "Weapon");
	}

	auto& animator = enemy.GetComponent<AnimatorComponent>();
	ClientSystems::PlayAnimation(&animator, ResourceManager::GetAnimation(idleAnimFile));
}

wstring GetTileTex(int type)
{
	switch (type)
	{
	case Grass:
		return TEXTURE(L"Cube_Pink.png");
		break;
	case Rail:
		return TEXTURE(L"Cube_SkyBlue.png");
		break;
	case Obstacle:
		return TEXTURE(L"Cube_Black.png");
		break;
	default:
		HB_ASSERT(false, "Invalid tile type: {0}", type);
		break;
	}

	return L"";
}
