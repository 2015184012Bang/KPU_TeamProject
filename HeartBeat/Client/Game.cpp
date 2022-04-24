#include "ClientPCH.h"
#include "Game.h"

#include "Components.h"
#include "Entity.h"
#include "Tags.h"

Game::Game()
	: mbRunning(true)
{

}

entt::entity Game::GetNewEntity()
{
	return mRegistry.create();
}

void Game::DestroyAll()
{
	mRegistry.each([this](entt::entity entity) {
		Entity e(entity, this);
		if (!e.HasComponent<Tag_DontDestroyOnLoad>())
		{
			mRegistry.destroy(entity);
		}
		});
}

void Game::DestroyEntityByID(const uint32 id)
{
	auto view = mRegistry.view<IDComponent>();

	for (auto [entity, idc] : view.each())
	{
		if (idc.ID == id)
		{
			mRegistry.destroy(entity);
		}
	}
}

void Game::DestroyEntity(const entt::entity entity)
{
	mRegistry.destroy(entity);
}

entt::entity Game::GetEntityByID(const uint32 id)
{
	auto view = mRegistry.view<IDComponent>();

	for (auto [entity, idc] : view.each())
	{
		if (idc.ID == id)
		{
			return entity;
		}
	}

	return entt::null;
}
