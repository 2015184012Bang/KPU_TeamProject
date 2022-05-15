#include "pch.h"
#include "Components.h"


IDComponent::IDComponent(const UINT32 id)
	: ID(id)
{

}


NameComponent::NameComponent(string_view name)
	: Name(name.data())
{

}


TransformComponent::TransformComponent(const Vector3& pos, const float yaw)
	: Position(pos)
	, Yaw(yaw)
{

}


MovementComponent::MovementComponent(const Vector3& dir, const float speed)
	: Direction(dir)
	, Speed(speed)
{

}


BoxComponent::BoxComponent(const Box* localBox, const Vector3& position, const float yaw)
	: LocalBox(localBox)
	, WorldBox(*localBox)
{
	WorldBox.Update(position, yaw);
}

HealthComponent::HealthComponent(const INT8 health)
	: Health(health)
{

}

ScriptComponent::ScriptComponent(shared_ptr<Script>&& script) noexcept
	: NativeScript(move(script))
{

}

SpawnComponent::SpawnComponent(EntityType eType, float spawnTime, float posX, float posZ)
	: EType(eType)
	, SpawnTime(spawnTime)
	, GenPosX(posX)
	, GenPosZ(posZ)
{

}

IHitYouComponent::IHitYouComponent(UINT32 hitter, UINT32 victim)
	: HitterID(hitter)
	, VictimID(victim)
{

}
