#include "ClientPCH.h"
#include "ResourceManager.h"

unordered_map<wstring, Mesh*> ResourceManager::sMeshes;
unordered_map<wstring, Texture*> ResourceManager::sTextures;
unordered_map<wstring, Skeleton*> ResourceManager::sSkeletons;
unordered_map<wstring, Animation*> ResourceManager::sAnimations;
unordered_map<wstring, AABB*> ResourceManager::sBoxes;

void ResourceManager::Shutdown()
{
	for (auto& [_, mesh]: sMeshes)
	{
		delete mesh;
	}
	sMeshes.clear();

	for (auto& [_, texture] : sTextures)
	{
		delete texture;
	}
	sTextures.clear();

	for (auto& [_, skel] : sSkeletons)
	{
		delete skel;
	}
	sSkeletons.clear();

	for (auto& [_, anim] : sAnimations)
	{
		delete anim;
	}
	sAnimations.clear();
}

Mesh* ResourceManager::GetMesh(const wstring& path)
{
	auto iter = sMeshes.find(path);

	if (iter != sMeshes.end())
	{
		return iter->second;
	}
	else
	{
		Mesh* newMesh = new Mesh;
		newMesh->Load(path);
		sMeshes[path] = newMesh;

		return newMesh;
	}
}

Texture* ResourceManager::GetTexture(const wstring& path)
{
	auto iter = sTextures.find(path);

	if (iter != sTextures.end())
	{
		return iter->second;
	}
	else
	{
		Texture* newTexture = new Texture;
		newTexture->Load(path);
		sTextures[path] = newTexture;

		return newTexture;
	}
}

Skeleton* ResourceManager::GetSkeleton(const wstring& path)
{
	auto iter = sSkeletons.find(path);
	
	if (iter != sSkeletons.end())
	{
		return iter->second;
	}
	else
	{
		Skeleton* newSkel = new Skeleton;
		newSkel->Load(path);
		sSkeletons[path] = newSkel;

		return newSkel;
	}
}

Animation* ResourceManager::GetAnimation(const wstring& path)
{
	auto iter = sAnimations.find(path);

	if (iter != sAnimations.end())
	{
		return iter->second;
	}
	else
	{
		Animation* newAnim = new Animation;
		newAnim->Load(path);
		sAnimations[path] = newAnim;

		return newAnim;
	}
}

AABB* ResourceManager::GetAABB(const wstring& path)
{
	auto iter = sBoxes.find(path);

	if (iter != sBoxes.end())
	{
		return iter->second;
	}
	else
	{
		AABB* newBox = new AABB;
		newBox->Load(path);
		sBoxes[path] = newBox;

		return newBox;
	}
}
