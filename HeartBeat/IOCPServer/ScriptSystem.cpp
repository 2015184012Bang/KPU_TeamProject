#include "pch.h"
#include "ScriptSystem.h"

#include "Script.h"
#include "Room.h"

ScriptSystem::ScriptSystem(entt::registry& registry, shared_ptr<Room>&& room)
	: mRegistry{ registry }
	, mOwner{ move(room) }
{

}

void ScriptSystem::Update()
{
	auto view = mRegistry.view<ScriptComponent>();

	for (auto [entity, script] : view.each())
	{
		if (!script.bInitialized)
		{
			script.bInitialized = true;
			script.NativeScript->Start();
		}

		script.NativeScript->Update();
	}
}
