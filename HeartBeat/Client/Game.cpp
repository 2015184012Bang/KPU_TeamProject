#include "ClientPCH.h"
#include "Game.h"

#include "ClientComponents.h"
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
			DestroyEntityByID(e.GetComponent<IDComponent>().ID);
		}
		});
}

void Game::DestroyEntityByID(const HBID& id)
{
	auto iter = mEntities.find(id);

	Entity e(iter->second, this);
	if (e.HasComponent<AttachmentParentComponent>())
	{
		DestroyEntityByID(e.GetComponent<AttachmentParentComponent>().ChildID);
	}

	if (iter != mEntities.end())
	{
		HB_LOG("Entity ID[{0}] Deleted!", id);
		mRegistry.destroy(iter->second);
		mEntities.erase(iter);
	}
	else
	{
		HB_LOG("There is no entity id[{0}]!", id);
	}
}

void Game::DestroyEntity(const entt::entity entity)
{
	Entity e(entity, this);
	DestroyEntityByID(e.GetComponent<IDComponent>().ID);
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
