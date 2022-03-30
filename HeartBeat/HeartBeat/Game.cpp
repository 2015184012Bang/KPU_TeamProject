#include "PCH.h"
#include "Game.h"

#include "Components.h"
#include "Entity.h"
#include "Tags.h"

Game::Game()
	: mbRunning(true)
{

}

void Game::RegisterEntity(const HBID& id, const entt::entity entity)
{
	auto iter = mEntities.find(id);

	if (iter == mEntities.end())
	{
		mEntities.emplace(id, entity);
	}
	else
	{
		HB_LOG("Entity ID[{0}] already exists!", id);
	}
}

entt::entity Game::getNewEntt()
{
	return mRegistry.create();
}

void Game::DestroyAll()
{
	mRegistry.each([this](entt::entity entity) {
		Entity e(entity, this);
		if (!e.HasComponent<Tag_DontDestroyOnLoad>())
		{
			DestroyEntity(entity);
		}
		});
}

void Game::DestroyEntity(const entt::entity handle)
{
	Entity e(handle, this);
	auto& idc = e.GetComponent<IDComponent>();
	removeEntity(idc.ID);

	mRegistry.destroy(handle);
}

void Game::DestroyEntityByID(const HBID& id)
{
	auto iter = mEntities.find(id);

	if (iter != mEntities.end())
	{
		DestroyEntity(iter->second);
	}
}

entt::entity Game::GetEntityByID(const HBID& id)
{
	auto iter = mEntities.find(id);

	if (iter != mEntities.end())
	{
		return iter->second;
	}
	else
	{
		return entt::null;
	}
}

void Game::removeEntity(const HBID& id)
{
	auto iter = mEntities.find(id);

	if (iter != mEntities.end())
	{
		HB_LOG("Entity ID[{0}] Deleted!", id);
		mEntities.erase(iter);
	}
	else
	{
		HB_LOG("There is no entity id[{0}]!", id);
	}
}
