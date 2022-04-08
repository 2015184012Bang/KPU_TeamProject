#include "ServerPCH.h"
#include "EnemyGenerator.h"

#include "rapidcsv.h"

#include "HeartBeat/PacketType.h"
#include "HeartBeat/Tags.h"
#include "HeartBeat/Random.h"

#include "Server.h"
#include "ServerComponents.h"


EnemyGenerator::EnemyGenerator(Server* server)
	: mServer(server)
{
	readStageFile("stage1.csv");
}

EnemyGenerator::~EnemyGenerator()
{

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
