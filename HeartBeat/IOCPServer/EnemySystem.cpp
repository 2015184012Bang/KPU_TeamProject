#include "pch.h"
#include "EnemySystem.h"

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
	string stageFile = "../Assets/Stages/Stage1.csv";
	rapidcsv::Document doc(stageFile, rapidcsv::LabelParams(-1, -1));
	mStages.emplace(stageFile, doc);
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

			if (EntityType::VIRUS == spawn.EType)
			{
				mRegistry.emplace<Tag_Virus>(entity);
			}
			else if (EntityType::DOG == spawn.EType)
			{
				mRegistry.emplace<Tag_Dog>(entity);
			}

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
}

void EnemySystem::Start(string_view fileName)
{
	mbGenerate = true;
	loadStageFile(fileName);
}

void EnemySystem::Reset()
{
	mbGenerate = false;
	DestroyByComponent<Tag_Enemy>(mRegistry);
}

void EnemySystem::loadStageFile(string_view fileName)
{
	if (auto iter = mStages.find(fileName.data()); iter != mStages.end())
	{
		const auto& doc = iter->second;

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
	else
	{
		LOG("There is no stage file: {0}", fileName.data());
	}
}
