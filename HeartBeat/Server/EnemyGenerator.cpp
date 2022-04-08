#include "ServerPCH.h"
#include "EnemyGenerator.h"

#include "HeartBeat/PacketType.h"
#include "HeartBeat/Tags.h"

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

		Entity virus = mServer->CreateEntity();
		virus.AddTag<Tag_Enemy>();
		auto& transform = virus.GetComponent<STransformComponent>();
		auto& id = virus.GetComponent<IDComponent>();

		MemoryStream* packet = new MemoryStream;
		packet->WriteUByte(static_cast<uint8>(SCPacket::eCreateEnemy));
		packet->WriteUInt64(id.ID);
		packet->WriteUByte(Virus);
		packet->WriteVector3(transform.Position);
		packet->WriteFloat(transform.Rotation.y);

		mServer->PushPacket(packet);
	}
}
