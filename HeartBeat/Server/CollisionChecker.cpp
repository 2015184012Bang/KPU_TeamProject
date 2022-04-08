#include "ServerPCH.h"
#include "CollisionChecker.h"

#include <rapidjson/document.h>

#include "HeartBeat/Tags.h"

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
}

CollisionChecker::~CollisionChecker()
{
	mLocalBoxes.clear();
}

void CollisionChecker::Update()
{
	auto view = mServer->GetRegistry().view<BoxComponent>();

	for (auto id : view)
	{
		Entity entity = Entity(id, mServer);

		auto& transform = entity.GetComponent<STransformComponent>();
		auto& box = entity.GetComponent<BoxComponent>();

		ServerSystems::UpdateBox(box.Local, &box.World, transform.Position, transform.Rotation.y);
	}

	auto players = mServer->GetRegistry().view<Tag_Player>();
	auto enemies = mServer->GetRegistry().view<Tag_Enemy>();

	for (auto player : players)
	{
		Entity p = Entity(player, mServer);
		auto& playerBox = p.GetComponent<BoxComponent>();

		for (auto enemy : enemies)
		{
			Entity e = Entity(enemy, mServer);
			auto& enemyBox = e.GetComponent<BoxComponent>();

			if (ServerSystems::Intersects(playerBox.World, enemyBox.World))
			{
				processCollision(e, p);
				p.AddTag<Tag_UpdateTransform>();
			}
		}
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
	float dy1 = aMax.y - bMin.y;
	float dy2 = aMin.y - bMax.y;
	float dz1 = aMax.z - bMin.z;
	float dz2 = aMin.z - bMax.z;

	float dx = abs(dx1) < abs(dx2) ?
		dx1 : dx2;
	float dy = abs(dy1) < abs(dy2) ?
		dy1 : dy2;
	float dz = abs(dz1) < abs(dz2) ?
		dz1 : dz2;

	STransformComponent& bTransform = b.GetComponent<STransformComponent>();

	if (abs(dx) <= abs(dy) && abs(dx) <= abs(dz))
	{
		bTransform.Position.x += dx * 2.0f;
	}
	else if (abs(dy) <= abs(dx) && abs(dy) <= abs(dz))
	{
		bTransform.Position.y += dy;
	}
	else
	{
		bTransform.Position.z += dz * 2.0f;
	}

	ServerSystems::UpdateBox(bBox.Local, &bBox.World, bTransform.Position, bTransform.Rotation.y);
}
