#include "PCH.h"
#include "Game.h"

Game::Game()
	: mbRunning(true)
{

}

entt::entity Game::CreateEntity()
{
	return mRegistry.create();
}

void Game::DestroyEntity(const entt::entity handle)
{
	mRegistry.destroy(handle);
}
