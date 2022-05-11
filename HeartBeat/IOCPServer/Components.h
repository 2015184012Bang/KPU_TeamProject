#pragma once

#include "Box.h"
#include "Protocol.h"

class Script;

struct IDComponent
{
	IDComponent() = default;
	IDComponent(const UINT32 id);

	UINT32 ID = UINT32_MAX;
};

struct NameComponent
{
	NameComponent() = default;
	NameComponent(string_view name);

	string Name = "default"s;
};

struct TransformComponent
{
	TransformComponent() = default;
	TransformComponent(const Vector3& pos, const float yaw);

	Vector3 Position = Vector3::Zero;
	float Yaw = 0.0f;
};

struct MovementComponent
{
	MovementComponent() = default;
	MovementComponent(const Vector3& dir, const float speed);

	Vector3 Direction = Vector3::Zero;
	float Speed = 0.0f;
};

struct CombatComponent
{
	CombatComponent() = default;
	CombatComponent(INT32 dmg, INT32 armor, 
		INT32 regen, const float baseAttackCooldown);

	INT32 BaseAttackDmg = 0;
	INT32 Armor = 0;
	INT32 Regeneration = 0;
	float BaseAttackCooldown = 0.0f;
	float BaseAttackTracker = 0.0f;
};

struct BoxComponent
{
	BoxComponent() = default;
	BoxComponent(const Box* localBox, const Vector3& position, const float yaw);

	const Box* LocalBox = nullptr;
	Box WorldBox = {};
};

struct HealthComponent
{
	HealthComponent() = default;
	HealthComponent(const UINT8 health);

	UINT8 Health = 0;
};

struct ScriptComponent
{
	ScriptComponent() = default;
	ScriptComponent(shared_ptr<Script>&& script) noexcept;

	shared_ptr<Script> NativeScript = nullptr;
	bool bInitialized = false;
};

struct SpawnComponent
{
	SpawnComponent() = default;
	SpawnComponent(EntityType eType, float spawnTime, float posX, float posZ);

	EntityType EType = EntityType::END;
	float SpawnTime = 0.0f;
	float GenPosX = 0.0f;
	float GenPosZ = 0.0f;
};

struct PathFindComponent
{
	PathFindComponent() = default;

	Vector3 MyPosition = Vector3::Zero;
	Vector3 TargetPosition = Vector3::Zero;

	// 길찾기 수행을 계속하는 지 여부
	bool bContinue = false;
};