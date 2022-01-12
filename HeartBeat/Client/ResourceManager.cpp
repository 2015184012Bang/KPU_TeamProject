#include "ClientPCH.h"
#include "ResourceManager.h"

unordered_map<wstring, Mesh*> ResourceManager::mMeshes;
unordered_map<wstring, Texture*> ResourceManager::mTextures;

void ResourceManager::Shutdown()
{
	for (auto& [_, mesh]: mMeshes)
	{
		delete mesh;
	}
	mMeshes.clear();

	for (auto& [_, texture] : mTextures)
	{
		delete texture;
	}
	mTextures.clear();
}

Mesh* ResourceManager::GetMesh(const wstring& path)
{
	auto iter = mMeshes.find(path);

	if (iter != mMeshes.end())
	{
		return iter->second;
	}
	else
	{
		Mesh* newMesh = new Mesh;
		newMesh->Load(path);
		mMeshes[path] = newMesh;

		return newMesh;
	}
}

Texture* ResourceManager::GetTexture(const wstring& path)
{
	auto iter = mTextures.find(path);

	if (iter != mTextures.end())
	{
		return iter->second;
	}
	else
	{
		Texture* newTexture = new Texture;
		newTexture->Load(path);
		mTextures[path] = newTexture;

		return newTexture;
	}
}
