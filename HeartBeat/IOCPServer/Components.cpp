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


CombatComponent::CombatComponent(INT32 dmg, INT32 armor, INT32 regen, const float baseAttackCooldown)
	: BaseAttackDmg(dmg)
	, Armor(armor)
	, Regeneration(regen)
	, BaseAttackCooldown(baseAttackCooldown)
{

}


BoxComponent::BoxComponent(const Box* localBox, const Vector3& position, const float yaw)
	: LocalBox(localBox)
	, WorldBox(*localBox)
{
	WorldBox.Update(position, yaw);
}
