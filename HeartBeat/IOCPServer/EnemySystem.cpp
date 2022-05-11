#include "pch.h"
#include "EnemySystem.h"

#include "rapidcsv.h"

#include "Entity.h"
#include "Components.h"
#include "Tags.h"
#include "Timer.h"
#include "Values.h"
#include "Box.h"
#include "Enemy.h"
#include "Protocol.h"
#include "Room.h"


EnemySystem::EnemySystem(entt::registry& registry, shared_ptr<Room>&& room)
	: mRegistry{ registry }
	, mOwner{ move(room) }
{
	
}

void EnemySystem::Update()
{
	if (!mbGenerate)
	{
		return;
	}

	auto view = mRegistry.view<SpawnComponent>();

	for (auto [entity, spawn] : view.each())
	{
		spawn.SpawnTime -= Timer::GetDeltaTime();

		if (spawn.SpawnTime < 0.0f)
		{
			auto& transform = mRegistry.emplace<TransformComponent>(entity, Vector3{ spawn.GenPosX, 0.0f, spawn.GenPosZ },
				0.0f);
			auto& id = mRegistry.emplace<IDComponent>(entity, mOwner->GetEntityID());
			
			mRegistry.emplace<MovementComponent>(entity, Vector3::Zero, Values::EnemySpeed);

			if (spawn.EType == EntityType::VIRUS)
			{
				mRegistry.emplace<BoxComponent>(entity, &Box::GetBox("../Assets/Boxes/Virus.box"), transform.Position, transform.Yaw);
			}
			else if (spawn.EType == EntityType::DOG)
			{
				mRegistry.emplace<BoxComponent>(entity, &Box::GetBox("../Assets/Boxes/Dog.box"), transform.Position, transform.Yaw);
			}
			else
			{
				ASSERT(false, "Unknown enemy type!");
			}

			mRegistry.emplace<HealthComponent>(entity, Values::EnemyHealth);
			mRegistry.emplace<ScriptComponent>(entity, make_shared<Enemy>(mRegistry, entity));
			mRegistry.emplace<Tag_Enemy>(entity);
			mRegistry.emplace<PathFindComponent>(entity);

			NOTIFY_CREATE_ENTITY_PACKET packet = {};
			packet.EntityID = id.ID;
			packet.EntityType = spawn.EType == EntityType::VIRUS ? static_cast<UINT8>(EntityType::VIRUS)
				: static_cast<UINT8>(EntityType::DOG);
			packet.PacketID = NOTIFY_CREATE_ENTITY;
			packet.PacketSize = sizeof(packet);
			packet.Position = transform.Position;
			mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));

			// 다시 생성되지 않도록 Spawn 컴포넌트 제거
			mRegistry.remove<SpawnComponent>(entity);
		}
	}

	//testDeletion();
}

void EnemySystem::LoadStageFile(string_view fileName)
{
	// 이전 스테이지에 생성했던 Enemy 제거
	DestroyByComponent<Tag_Enemy>(mRegistry);

	// 새로운 스테이지 파일 읽고 Enemy 생성
	readStageFile(fileName);
}

void EnemySystem::readStageFile(string_view fileName)
{
	rapidcsv::Document doc(fileName.data(), rapidcsv::LabelParams(-1, -1));

	for (size_t i = 0; i < doc.GetRowCount(); ++i)
	{
		vector<string> parsed = doc.GetRow<string>(i);

		auto enemy = mRegistry.create();

		// 생성 시간 체크를 위해 SpawnComponent만 부착한다.
		EntityType eType = parsed[0] == "Virus" ? EntityType::VIRUS
			: EntityType::DOG;

		mRegistry.emplace<SpawnComponent>(enemy, 
			eType,
			stof(parsed[1]),
			stof(parsed[2]),
			stof(parsed[3]));
	}
}

void EnemySystem::testDeletion()
{
	static float elapsed = 0.0f;
	elapsed += Timer::GetDeltaTime();
	if (elapsed > 15.0f)
	{
		auto view = mRegistry.view<SpawnComponent, IDComponent>();

		for (auto [entity, spawn, id] : view.each())
		{
			NOTIFY_DELETE_ENTITY_PACKET packet = {};
			packet.EntityID = id.ID;
			packet.EntityType = static_cast<UINT8>(spawn.EType);
			packet.PacketID = NOTIFY_DELETE_ENTITY;
			packet.PacketSize = sizeof(packet);
			mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
			DestroyEntity(mRegistry, entity);
		}
	}
}
