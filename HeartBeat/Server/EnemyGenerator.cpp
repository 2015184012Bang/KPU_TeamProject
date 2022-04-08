#include "ServerPCH.h"
#include "EnemyGenerator.h"

#include "rapidcsv.h"

#include "HeartBeat/PacketType.h"
#include "HeartBeat/Tags.h"
#include "HeartBeat/Random.h"
#include "HeartBeat/Define.h"

#include "CollisionChecker.h"
#include "Server.h"
#include "ServerComponents.h"


EnemyGenerator::EnemyGenerator(Server* server)
	: mServer(server)
{
	readStageFile("../Assets/Stages/stage1.csv");

	mCollsionChecker = server->GetCollisionChecker();
}

EnemyGenerator::~EnemyGenerator()
{
	mCollsionChecker = nullptr;
}

void EnemyGenerator::Update()
{
	if (!mbStart)
	{
		return;
	}

	mElapsed += Timer::GetDeltaTime();

	while(!mSpawnInfos.empty())
	{
		const auto& info = mSpawnInfos.top();
		if (mElapsed > info.SpawnTime)
		{
			Entity enemy = mServer->CreateEntity();
			enemy.AddTag<Tag_Enemy>();
			auto& transform = enemy.GetComponent<STransformComponent>();
			transform.Position.x = info.X;
			transform.Position.z = info.Z;

			// TODO: 적의 타입에 따라 다른 박스 설정 필요
			enemy.AddComponent<SBoxComponent>(mCollsionChecker->GetLocalBox(L"Virus"), transform.Position);
			auto& id = enemy.GetComponent<IDComponent>();

			MemoryStream* packet = new MemoryStream;
			packet->WriteUByte(static_cast<uint8>(SCPacket::eCreateEnemy));
			packet->WriteUInt64(id.ID);
			packet->WriteUByte(getEnemyType(info.Type));
			packet->WriteVector3(transform.Position);
			mServer->PushPacket(packet);

			mSpawnInfos.pop();
		}
		else
		{
			break;
		}
	}
}

void EnemyGenerator::readStageFile(const string& stageFile)
{
	rapidcsv::Document doc(stageFile, rapidcsv::LabelParams(-1, -1));

	for (int i = 0; i < doc.GetRowCount(); ++i)
	{
		std::vector<string> parsed = doc.GetRow<string>(i);

		mSpawnInfos.emplace(
			parsed[0],			// Enemy Type
			stof(parsed[1]),	// Spawn Time
			stof(parsed[2]),	// X position
			stof(parsed[3]));	// Z position
	}
}

uint8 EnemyGenerator::getEnemyType(const string& name)
{
	if ("Virus" == name)
	{
		return Virus;
	}
	else if ("Dog" == name)
	{
		return Dog;
	}
	else
	{
		HB_ASSERT(false, "No Enemy Type: {0}", name);
	}
}
