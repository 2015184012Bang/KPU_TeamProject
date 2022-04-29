#include "ClientPCH.h"
#include "GameScene.h"

#include "Client.h"
#include "Components.h"
#include "GameMap.h"
#include "PacketManager.h"
#include "Input.h"
#include "Random.h"
#include "ResourceManager.h"

GameScene::GameScene(Client* owner)
	: Scene(owner)
{

}

void GameScene::Enter()
{
	// 내 캐릭터 알아두기
	auto entity = mOwner->GetEntityByID(mOwner->GetClientID());
	mPlayerCharacter = Entity(entity, mOwner);

	// 맵 생성
	createMap();
}

void GameScene::Exit()
{

}

void GameScene::ProcessInput()
{
	PACKET packet;
	while (mOwner->GetPacketManager()->GetPacket(packet))
	{
		switch (packet.PacketID)
		{
		case ANSWER_MOVE:
			processAnswerMove(packet);
			break;

		case NOTIFY_MOVE:
			processNotifyMove(packet);
			break;

		case ANSWER_ATTACK:
			processAnswerAttack(packet);
			break;

		case NOTIFY_ATTACK:
			processNotifyAttack(packet);
			break;

		default:
			HB_LOG("Unknown packet id: {0}", packet.PacketID);
			break;
		}

		if (mbChangeScene)
		{
			// TODO: 게임 종료 씬으로 전환
			break;
		}
	}
}

void GameScene::Update(float deltaTime)
{
	bool isKeyPressed = pollKeyboardPressed();
	bool isKeyReleased = pollKeyboardReleased();

	if (isKeyPressed || isKeyReleased)
	{
		REQUEST_MOVE_PACKET packet = {};
		packet.PacketID = REQUEST_MOVE;
		packet.PacketSize = sizeof(packet);
		packet.Direction = mDirection;
		mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), sizeof(packet));
	}

	if (Input::IsButtonPressed(KeyCode::A))
	{
		REQUEST_ATTACK_PACKET packet = {};
		packet.PacketID = REQUEST_ATTACK;
		packet.PacketSize = sizeof(packet);

		mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), sizeof(packet));
	}
}


void GameScene::createMap()
{
	const auto& tiles = gGameMap.GetTiles();

	for (const auto& tile : tiles)
	{
		const Texture* tileTex = GetTileTexture(tile.TType);

		Entity obj = mOwner->CreateStaticMeshEntity(MESH("Cube.mesh"),
			tileTex);

		auto& transform = obj.GetComponent<TransformComponent>();
		transform.Position.x = tile.X;
		transform.Position.y = -TILE_WIDTH;
		transform.Position.z = tile.Z;
	}
}

bool GameScene::pollKeyboardPressed()
{
	bool bChanged = false;

	if (Input::IsButtonPressed(KeyCode::LEFT))
	{
		mDirection.x -= 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonPressed(KeyCode::RIGHT))
	{
		mDirection.x += 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonPressed(KeyCode::UP))
	{
		mDirection.z += 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonPressed(KeyCode::DOWN))
	{
		mDirection.z -= 1.0f;
		bChanged = true;
	}

	return bChanged;
}

bool GameScene::pollKeyboardReleased()
{
	bool bChanged = false;

	if (Input::IsButtonReleased(KeyCode::LEFT))
	{
		mDirection.x += 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonReleased(KeyCode::RIGHT))
	{
		mDirection.x -= 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonReleased(KeyCode::UP))
	{
		mDirection.z -= 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonReleased(KeyCode::DOWN))
	{
		mDirection.z += 1.0f;
		bChanged = true;
	}

	return bChanged;
}

void GameScene::processAnswerMove(const PACKET& packet)
{
	ANSWER_MOVE_PACKET* amPacket = reinterpret_cast<ANSWER_MOVE_PACKET*>(packet.DataPtr);

	auto& transform = mPlayerCharacter.GetComponent<TransformComponent>();
	transform.Position = amPacket->Position;

	auto& movement = mPlayerCharacter.GetComponent<MovementComponent>();
	movement.Direction = amPacket->Direction;
}

void GameScene::processNotifyMove(const PACKET& packet)
{
	NOTIFY_MOVE_PACKET* nmPacket = reinterpret_cast<NOTIFY_MOVE_PACKET*>(packet.DataPtr);

	auto entity = mOwner->GetEntityByID(nmPacket->EntityID);

	Entity target = { entity, mOwner };

	auto& transform = target.GetComponent<TransformComponent>();
	transform.Position = nmPacket->Position;

	auto& movement = target.GetComponent<MovementComponent>();
	movement.Direction = nmPacket->Direction;
}

void GameScene::processAnswerAttack(const PACKET& packet)
{
	ANSWER_ATTACK_PACKET* aaPacket = reinterpret_cast<ANSWER_ATTACK_PACKET*>(packet.DataPtr);

	if (aaPacket->Result == ERROR_CODE::ATTACK_NOT_YET)
	{
		return;
	}

	auto& animator = mPlayerCharacter.GetComponent<AnimatorComponent>();
	animator.SetTrigger(GetRandomAttackAnimFile());
}

void GameScene::processNotifyAttack(const PACKET& packet)
{
	NOTIFY_ATTACK_PACKET* naPacket = reinterpret_cast<NOTIFY_ATTACK_PACKET*>(packet.DataPtr);

	auto entity = mOwner->GetEntityByID(naPacket->EntityID);
	Entity e = { entity, mOwner };

	auto& animator = e.GetComponent<AnimatorComponent>();
	animator.SetTrigger(GetRandomAttackAnimFile());
}

string GetRandomAttackAnimFile(bool isEnemy /*= false*/)
{
	if (isEnemy)
	{
		return "Attack";
	}
	else
	{
		switch (Random::RandInt(1, 3))
		{
		case 1:
			return "Attack1";
			break;

		case 2:
			return "Attack2";
			break;

		case 3:
			return "Attack3";
			break;

		default:
			return "";
			break;
		}
	}
}

Texture* GetTileTexture(TileType ttype)
{
	switch (ttype)
	{
	case TileType::BLOCKED:
		return TEXTURE("Black.png");

	case TileType::MOVABLE:
		return TEXTURE("LightGreen.png");

	case TileType::RAIL:
		return TEXTURE("Brown.png");

	case TileType::FAT:
		return TEXTURE("Pink.png");
		
	case TileType::TANK_FAT:
		return TEXTURE("Orange.png");
		
	case TileType::SCAR:
		return TEXTURE("Red.png");
		
	default:
		HB_ASSERT(false, "Unknown tile type!");
		return nullptr;
	}
}
