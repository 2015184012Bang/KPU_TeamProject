#include "ServerPCH.h"
#include "AIController.h"

#include "HeartBeat/Script.h"

#include "Server.h"
#include "ServerComponents.h"
#include "Tank.h"

AIController::AIController(Server* server)
	: mServer(server)
{
	
}

AIController::~AIController()
{

}

void AIController::Update()
{
	auto view = mServer->GetRegistry().view<AIComponent>();

	for (auto [entity, ai] : view.each())
	{
		if (!ai.bInitialized)
		{
			ai.AIScript->Start();
			ai.bInitialized = true;
		}

		ai.AIScript->Update(Timer::GetDeltaTime());
	}
}
