#include "ServerPCH.h"
#include "CollisionChecker.h"

#include <rapidjson/document.h>

#include "HeartBeat/Tags.h"
#include "HeartBeat/PacketType.h"

#include "Server.h"
#include "ServerComponents.h"
#include "ServerSystems.h"

/************************************************************************/
/* CollisionChecker                                                     */
/************************************************************************/

CollisionChecker::CollisionChecker(Server* server)
	: mServer(server)
{
	mLocalBoxes.try_emplace(L"Cell", BOX(L"Cell.box"));
	mLocalBoxes.try_emplace(L"Cube", BOX(L"Cube.box"));
	mLocalBoxes.try_emplace(L"Character", BOX(L"Character.box"));
	mLocalBoxes.try_emplace(L"Virus", BOX(L"Virus.box"));

	AABB hitBox;
	hitBox.SetMin(Vector3(-100.0f, 0.0f, 0.0f));
	hitBox.SetMax(Vector3(100.0f, 500.0f, 200.0f));

	mLocalBoxes.try_emplace(L"HitBox", hitBox);
}

CollisionChecker::~CollisionChecker()
{
	mLocalBoxes.clear();
}

void CollisionChecker::Update()
{
	auto view = mServer->GetRegistry().view<BoxComponent, STransformComponent>();

	for (auto [entity, box, transform] : view.each())
	{
		ServerSystems::UpdateBox(box.Local, &box.World, transform.Position, transform.Rotation.y);
	}

	auto players = mServer->GetRegistry().view<Tag_Player>();
	auto enemies = mServer->GetRegistry().view<Tag_Enemy>();

	vector<Entity> plrs;
	for (auto pEntity : players)
	{
		Entity player = Entity(pEntity, mServer);
		plrs.push_back(player);
		auto& playerBox = player.GetComponent<BoxComponent>();

		for (auto eEntity : enemies)
		{
			Entity enemy = Entity(eEntity, mServer);
			auto& enemyBox = enemy.GetComponent<BoxComponent>();

			if (ServerSystems::Intersects(playerBox.World, enemyBox.World))
			{
				processCollision(enemy, player);
				player.AddTag<Tag_UpdateTransform>();
			}
		}
	}

	if (plrs.size() >= 2)
	{
		for (int i = 0; i < plrs.size() - 1; ++i)
		{
			auto& p1Box = plrs[i].GetComponent<BoxComponent>();

			for (int j = i + 1; j < plrs.size(); ++j)
			{
				auto& p2Box = plrs[j].GetComponent<BoxComponent>();

				if (ServerSystems::Intersects(p1Box.World, p2Box.World))
				{
					processCollision(plrs[j], plrs[i]);
					plrs[i].AddTag<Tag_UpdateTransform>();
				}
			}
		}
	}
}

void CollisionChecker::MakeHitBoxAndCheck(const Vector3& position, float yaw)
{
	AABB hitBox;
	ServerSystems::UpdateBox(&mLocalBoxes[L"HitBox"], &hitBox, position, yaw);

	{
		auto view = mServer->GetRegistry().view<Tag_Enemy>();
		for (auto entity : view)
		{
			Entity enemy = Entity(entity, mServer);

			auto& box = enemy.GetComponent<BoxComponent>();

			if (ServerSystems::Intersects(hitBox, box.World))
			{
				auto& health = enemy.GetComponent<HealthComponent>();
				health.Health -= 1;

				// 만약 체력이 0이하라면 Dead 태그를 붙여둔다
				if (health.Health <= 0)
				{
					enemy.AddTag<Tag_Dead>();
				}

				break;
			}
		}
	}

	{
		auto view = mServer->GetRegistry().view<Tag_Dead>();

		if (view.empty())
		{
			return;
		}

		MemoryStream* packet = new MemoryStream;

		for (auto id : view)
		{
			Entity dead = Entity(id, mServer);

			auto& idComponent = dead.GetComponent<IDComponent>();

			packet->WriteUByte(static_cast<uint8>(SCPacket::eDeleteEntity));
			packet->WriteUInt64(idComponent.ID);
		}

		mServer->PushPacket(packet);

		// 죽은 개체들을 삭제한다
		mServer->DestroyByComponent<Tag_Dead>();
	}
}

const AABB* CollisionChecker::GetLocalBox(const wstring& name) const
{
	auto iter = mLocalBoxes.find(name);

	if (iter != mLocalBoxes.end())
	{
		return &iter->second;
	}
	else
	{
		HB_LOG("No box : {0}", ws2s(name));
		return  nullptr;
	}
}

void CollisionChecker::processCollision(Entity& a, Entity& b)
{
	auto& aBox = a.GetComponent<BoxComponent>();
	auto& bBox = b.GetComponent<BoxComponent>();

	const Vector3& aMax = aBox.World.GetMax();
	const Vector3& aMin = aBox.World.GetMin();

	const Vector3& bMax = bBox.World.GetMax();
	const Vector3& bMin = bBox.World.GetMin();

	float dx1 = aMax.x - bMin.x;
	float dx2 = aMin.x - bMax.x;
	float dz1 = aMax.z - bMin.z;
	float dz2 = aMin.z - bMax.z;

	float dx = abs(dx1) < abs(dx2) ?
		dx1 : dx2;
	float dz = abs(dz1) < abs(dz2) ?
		dz1 : dz2;

	STransformComponent& bTransform = b.GetComponent<STransformComponent>();

	if (abs(dx) <= abs(dz))
	{
		bTransform.Position.x += dx * 2.0f;
	}
	else
	{
		bTransform.Position.z += dz * 2.0f;
	}

	ServerSystems::UpdateBox(bBox.Local, &bBox.World, bTransform.Position, bTransform.Rotation.y);
}
