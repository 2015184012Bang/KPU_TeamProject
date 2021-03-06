#include "pch.h"
#include "EnemySystem.h"

#include "Entity.h"
#include "Components.h"
#include "Tags.h"
#include "Timer.h"
#include "Values.h"
#include "Box.h"
#include "Enemy.h"
#include "Room.h"
#include "Random.h"


EnemySystem::EnemySystem(entt::registry& registry, shared_ptr<Room>&& room)
	: mRegistry{ registry }
	, mOwner{ move(room) }
{
	string stageFile = "../Assets/Stages/Stage.csv";
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
			Vector3 genPos = Vector3{ spawn.GenPosX, 0.0f, spawn.GenPosZ };

			createEnemy(genPos, spawn.EType, entity);

			NOTIFY_CREATE_ENTITY_PACKET packet = {};
			packet.EntityID = mRegistry.get<IDComponent>(entity).ID;
			packet.EntityType = static_cast<UINT8>(spawn.EType);
			packet.PacketID = NOTIFY_CREATE_ENTITY;
			packet.PacketSize = sizeof(packet);
			packet.Position = mRegistry.get<TransformComponent>(entity).Position;
			mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));

			// 다시 생성되지 않도록 Spawn 컴포넌트 제거
			mRegistry.remove<SpawnComponent>(entity);
		}
	}
}

void EnemySystem::Start(string_view fileName)
{
	mbGenerate = true;
	mDuration = 0.0f;
	loadStageFile(fileName);
}

void EnemySystem::Reset()
{
	mbGenerate = false;
	DestroyByComponent<Tag_Enemy>(mRegistry);
}

void EnemySystem::GenerateEnemyMidBattle(const Vector3& controlPoint)
{
	Vector3 pos = controlPoint;
	pos.x = controlPoint.x - 2400.0f;

	const INT32 enemyCount = 7;

	for (int i = 0; i < enemyCount; ++i)
	{
		pos.z = (i * 400.0f) + 800.0f;
		auto randInt = Random::RandInt(0, 1);
		EntityType eType = randInt == 0 ? EntityType::VIRUS : EntityType::DOG;
		entt::entity entity = createEnemy(pos, eType);

		NOTIFY_CREATE_ENTITY_PACKET packet = {};
		packet.EntityID = mRegistry.get<IDComponent>(entity).ID;
		packet.EntityType = static_cast<UINT8>(eType);
		packet.PacketID = NOTIFY_CREATE_ENTITY;
		packet.PacketSize = sizeof(packet);
		packet.Position = pos;
		mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
	}

	pos.x = controlPoint.x + 2400.0f;
	for (int i = 0; i < enemyCount; ++i)
	{
		pos.z = (i * 400.0f) + 800.0f;
		auto randInt = Random::RandInt(0, 1);
		EntityType eType = randInt == 0 ? EntityType::VIRUS : EntityType::DOG;
		entt::entity entity = createEnemy(pos, eType);

		NOTIFY_CREATE_ENTITY_PACKET packet = {};
		packet.EntityID = mRegistry.get<IDComponent>(entity).ID;
		packet.EntityType = static_cast<UINT8>(eType);
		packet.PacketID = NOTIFY_CREATE_ENTITY;
		packet.PacketSize = sizeof(packet);
		packet.Position = pos;
		mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
	}
}

void EnemySystem::GenerateEnemyBossBattle(const Vector3& controlPoint)
{
	Vector3 pos = controlPoint;
	pos.x -= 2400.0f;
	pos.z -= 800.0f;

	const INT32 virusCount = 5;

	for (auto i = 0; i < virusCount; ++i)
	{
		pos.z += 400.0f;
		entt::entity entity = createEnemy(pos, EntityType::VIRUS);

		NOTIFY_CREATE_ENTITY_PACKET packet = {};
		packet.EntityID = mRegistry.get<IDComponent>(entity).ID;
		packet.EntityType = static_cast<UINT8>(EntityType::VIRUS);
		packet.PacketID = NOTIFY_CREATE_ENTITY;
		packet.PacketSize = sizeof(packet);
		packet.Position = pos;
		mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
	}
}

entt::entity EnemySystem::GenerateBoss()
{
	auto bossScar = GetEntityByName(mRegistry, "BossSpawnPoint");
	const auto& bsPosition = mRegistry.get<TransformComponent>(bossScar).Position;
	auto position = Vector3{ bsPosition.x, 0.0f, bsPosition.z };

	auto entity = createEnemy(position, EntityType::BOSS);

	// 본격적인 전투 시작 전엔 보스 무적 상태로 만듦.
	mRegistry.emplace<Tag_Invincible>(entity);

	return entity;
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

entt::entity EnemySystem::createEnemy(const Vector3& position, EntityType eType, entt::entity entity /*= entt::null*/)
{
	if (entity == entt::null)
	{
		entity = mRegistry.create();
	}
	
	auto& transform = mRegistry.emplace<TransformComponent>(entity, 
		position,
		0.0f);
	auto& id = mRegistry.emplace<IDComponent>(entity, mOwner->GetEntityID());
	mRegistry.emplace<Tag_Enemy>(entity);

	if (eType == EntityType::VIRUS)
	{
		mRegistry.emplace<BoxComponent>(entity, &Box::GetBox("../Assets/Boxes/Virus.box"), transform.Position, transform.Yaw);
		mRegistry.emplace<Tag_Virus>(entity);
		mRegistry.emplace<HealthComponent>(entity, Values::VirusHealth);
		mRegistry.emplace<MovementComponent>(entity, Vector3::Zero, Values::EnemySpeed);
		mRegistry.emplace<ScriptComponent>(entity, make_shared<Enemy>(mRegistry, entity));
		mRegistry.emplace<PathFindComponent>(entity);
	}
	else if (eType == EntityType::DOG)
	{
		mRegistry.emplace<BoxComponent>(entity, &Box::GetBox("../Assets/Boxes/Dog.box"), transform.Position, transform.Yaw);
		mRegistry.emplace<Tag_Dog>(entity);
		mRegistry.emplace<HealthComponent>(entity, Values::DogHealth);
		mRegistry.emplace<MovementComponent>(entity, Vector3::Zero, Values::EnemySpeed);
		mRegistry.emplace<ScriptComponent>(entity, make_shared<Enemy>(mRegistry, entity));
		mRegistry.emplace<PathFindComponent>(entity);
	}
	else if(eType == EntityType::BOSS)
	{     
		transform.Yaw = 270.0f;
		mRegistry.emplace<BoxComponent>(entity, &Box::GetBox("../Assets/Boxes/Boss.box"), 
			transform.Position, transform.Yaw);
		mRegistry.emplace<Tag_Boss>(entity);
		mRegistry.emplace<HealthComponent>(entity, Values::BossHealth);
		mRegistry.emplace<MovementComponent>(entity, Vector3::Zero, 0.0f);
		mRegistry.emplace<NameComponent>(entity, "Boss");
	}
	else
	{
		ASSERT(false, "Unknown enemy type!");
	}

	return entity;
}
