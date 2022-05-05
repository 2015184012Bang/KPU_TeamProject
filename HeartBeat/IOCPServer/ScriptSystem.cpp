#include "pch.h"
#include "ScriptSystem.h"

#include "GameManager.h"
#include "Script.h"

ScriptSystem::ScriptSystem(shared_ptr<GameManager>&& gm)
	: mGameManager(move(gm))
{

}

void ScriptSystem::Update()
{
	auto view = gRegistry.view<ScriptComponent>();

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
