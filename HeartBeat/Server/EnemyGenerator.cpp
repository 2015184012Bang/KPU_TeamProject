#include "ServerPCH.h"
#include "EnemyGenerator.h"

#include "HeartBeat/PacketType.h"
#include "HeartBeat/Tags.h"
#include "HeartBeat/Random.h"

#include "Server.h"
#include "ServerComponents.h"


EnemyGenerator::EnemyGenerator(Server* server)
	: mServer(server)
{

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

	if (mElapsed > mSpawnInterval)
	{
		mElapsed = 0.0f;

		Entity enemy = mServer->CreateEntity();
		enemy.AddTag<Tag_Enemy>();
		auto& transform = enemy.GetComponent<STransformComponent>();
		auto& id = enemy.GetComponent<IDComponent>();

		MemoryStream* packet = new MemoryStream;
		packet->WriteUByte(static_cast<uint8>(SCPacket::eCreateEnemy));
		packet->WriteUInt64(id.ID);
		packet->WriteUByte(Random::RandInt(0, 1));
		packet->WriteVector3(transform.Position);
		packet->WriteFloat(transform.Rotation.y);

		mServer->PushPacket(packet);
	}
}
