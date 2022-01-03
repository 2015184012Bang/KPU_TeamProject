#include "PCH.h"
#include "Game.h"

Game::Game()
	: mIsRunning(true)
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
