#pragma once

#include "Script.h"
#include "Components.h"
#include "Helpers.h"
#include "ResourceManager.h"

class RedCell : public Script
{
public:
	RedCell(Entity owner)
		: Script(owner)
	{

	}

	virtual void Start() override
	{
		mWeapons[0] = { MESH("CO2.mesh"), TEXTURE("CO2.png") };
		mWeapons[1] = { MESH("O2.mesh"), TEXTURE("O2.png") };
	}

	virtual void Update(float deltaTime) override
	{
		auto& movement = GetComponent<MovementComponent>();
		auto& animator = GetComponent<AnimatorComponent>();

		if (movement.Direction == Vector3::Zero && !bSwitch)
		{
			bSwitch = true;

			animator.SetTrigger("Idle");
			Helpers::DetachBone(GetOwner());
			Entity weapon = Entity{ gRegistry.create() };
			auto& transform = weapon.AddComponent<TransformComponent>();
			weapon.AddTag<Tag_StaticMesh>();
			weapon.AddComponent<MeshRendererComponent>(mWeapons[mCurrentIndex].first, mWeapons[mCurrentIndex].second);
			Helpers::AttachBone(GetOwner(), weapon, "Weapon");

			mCurrentIndex = (mCurrentIndex + 1) % 2;
		}
		else if(movement.Direction != Vector3::Zero)
		{
			bSwitch = false;

			animator.SetTrigger("Run");
		}
	}

private:
	array<std::pair<Mesh*, Texture*>, 2> mWeapons;
	int32 mCurrentIndex = 0;

	bool bSwitch = false;
};