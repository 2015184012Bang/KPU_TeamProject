#include "pch.h"
#include "EnemySystem.h"

#include "rapidcsv.h"

#include "GameManager.h"
#include "Entity.h"
#include "Components.h"
#include "Tags.h"
#include "Timer.h"
#include "Values.h"
#include "Box.h"
#include "Enemy.h"
#include "Protocol.h"

EnemySystem::EnemySystem(shared_ptr<GameManager>&& gm)
	: mGameManager(move(gm))
{
	readStageFile("../Assets/Stages/Stage1.csv");
}

void EnemySystem::Update()
{
	if (!mbGenerate)
	{
		return;
	}

	auto view = gRegistry.view<SpawnComponent>();

	for (auto [entity, spawn] : view.each())
	{
		spawn.SpawnTime -= Timer::GetDeltaTime();

		if (spawn.SpawnTime < 0.0f)
		{
			Entity enemy = Entity{ entity };
			auto& transform = enemy.AddComponent<TransformComponent>(Vector3{ spawn.GenPosX, 0.0f, spawn.GenPosZ },
				0.0f);
			auto& id = enemy.AddComponent<IDComponent>(Values::EntityID++);
			enemy.AddComponent<MovementComponent>(Vector3::Zero, Values::EnemySpeed);
			
			if (spawn.EType == EntityType::VIRUS)
			{
				enemy.AddComponent<BoxComponent>(&Box::GetBox("../Assets/Boxes/Virus.box"), transform.Position, transform.Yaw);
			}
			else if (spawn.EType == EntityType::DOG)
			{
				enemy.AddComponent<BoxComponent>(&Box::GetBox("../Assets/Boxes/Dog.box"), transform.Position, transform.Yaw);
			}
			else
			{
				ASSERT(false, "Unknown enemy type!");
			}

			enemy.AddComponent<HealthComponent>(Values::EnemyHealth);
			enemy.AddComponent<ScriptComponent>(make_shared<Enemy>(enemy));
			enemy.AddTag<Tag_Enemy>();

			NOTIFY_CREATE_ENTITY_PACKET packet = {};
			packet.EntityID = id.ID;
			packet.EntityType = spawn.EType == EntityType::VIRUS ? static_cast<UINT8>(EntityType::VIRUS) 
				: static_cast<UINT8>(EntityType::DOG);
			packet.PacketID = NOTIFY_CREATE_ENTITY;
			packet.PacketSize = sizeof(packet);
			packet.Position = transform.Position;

			// 생성한 적은 SpawnComponent 제거.
			enemy.RemoveComponent<SpawnComponent>();

			mGameManager->SendToAll(sizeof(packet), reinterpret_cast<char*>(&packet));
		}
	}
}

void EnemySystem::LoadStageFile(string_view fileName)
{
	// 이전 스테이지에 생성했던 Enemy 제거
	DestroyByComponent<Tag_Enemy>();
	
	// 새로운 스테이지 파일 읽고 Enemy 생성
	readStageFile(fileName);
}

void EnemySystem::readStageFile(string_view fileName)
{
	rapidcsv::Document doc(fileName.data(), rapidcsv::LabelParams(-1, -1));

	for (size_t i = 0; i < doc.GetRowCount(); ++i)
	{
		vector<string> parsed = doc.GetRow<string>(i);

		Entity enemy = Entity{ gRegistry.create() };

		// 생성 시간 체크를 위해 SpawnComponent만 부착한다.
		EntityType eType = parsed[0] == "Virus" ? EntityType::VIRUS
			: EntityType::DOG;

		enemy.AddComponent<SpawnComponent>(
			eType,
			stof(parsed[1]),
			stof(parsed[2]),
			stof(parsed[3]));
	}
}
