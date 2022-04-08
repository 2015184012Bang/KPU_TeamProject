#include "ClientPCH.h"
#include "ResourceManager.h"

#include "HeartBeat/Define.h"

#include "AABB.h"
#include "Animation.h"
#include "Font.h"
#include "Mesh.h"
#include "Skeleton.h"
#include "Texture.h"

unordered_map<wstring, Mesh*> ResourceManager::sMeshes;
unordered_map<wstring, Texture*> ResourceManager::sTextures;
unordered_map<wstring, Skeleton*> ResourceManager::sSkeletons;
unordered_map<wstring, Animation*> ResourceManager::sAnimations;
unordered_map<wstring, AABB*> ResourceManager::sBoxes;
unordered_map<wstring, Mesh*> ResourceManager::sDebugMeshes;
unordered_map<wstring, Font*> ResourceManager::sFonts;

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

	for (auto& [_, box] : sBoxes)
	{
		delete box;
	}
	sBoxes.clear();

	for (auto& [_, debugMesh] : sDebugMeshes)
	{
		delete debugMesh;
	}
	sDebugMeshes.clear();

	for (auto& [_, font] : sFonts)
	{
		delete font;
	}
	sFonts.clear();
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

		Mesh* newDebugMesh = new Mesh;
		newDebugMesh->LoadDebugMesh(newBox->GetMin(), newBox->GetMax());
		sDebugMeshes[path] = newDebugMesh;

		return newBox;
	}
}

Mesh* ResourceManager::GetDebugMesh(const wstring& path)
{
	auto iter = sDebugMeshes.find(path);

	if (iter != sDebugMeshes.end())
	{
		return iter->second;
	}
	else
	{
		HB_LOG("Debug mesh not found: {0}", ws2s(path));
		HB_ASSERT(false, "ASSERTION FAILED");
	}

	return nullptr;
}

Font* ResourceManager::GetFont(const wstring& path)
{
	auto iter = sFonts.find(path);

	if (iter != sFonts.end())
	{
		return iter->second;
	}
	else
	{
		Font* newFont = new Font;
		newFont->Load(path);
		sFonts[path] = newFont;

		return newFont;
	}
}


void GetCharacterFiles(int clientID, wstring* outMeshFile, wstring* outTexFile, wstring* outSkelFile)
{
	switch (clientID)
	{
	case 0:
		*outMeshFile = L"Assets/Meshes/Character_Green.mesh";
		*outTexFile = L"Assets/Textures/Character_Green.png";
		*outSkelFile = L"Assets/Skeletons/Character_Green.skel";
		break;

	case 1:
		*outMeshFile = L"Assets/Meshes/Character_Pink.mesh";
		*outTexFile = L"Assets/Textures/Character_Pink.png";
		*outSkelFile = L"Assets/Skeletons/Character_Pink.skel";
		break;

	case 2:
		*outMeshFile = L"Assets/Meshes/Character_Red.mesh";
		*outTexFile = L"Assets/Textures/Character_Red.png";
		*outSkelFile = L"Assets/Skeletons/Character_Red.skel";
		break;

	default:
		HB_ASSERT(false, "Unknown client id: {0}", clientID);
		break;
	}
}

wstring GetCharacterAnimation(int clientID, CharacterAnimationType type)
{
	wstring animFile;

	switch (clientID)
	{
	case 0: // Character_Green
		switch (type)
		{
		case CharacterAnimationType::eIdle:
			animFile = L"Assets/Animations/CG_Idle.anim";
			break;

		case CharacterAnimationType::eRun:
			animFile = L"Assets/Animations/CG_Run.anim";
			break;
		}
		break;

	case 1: // Character_Pink
		switch (type)
		{
		case CharacterAnimationType::eIdle:
			animFile = L"Assets/Animations/CP_Idle.anim";
			break;

		case CharacterAnimationType::eRun:
			animFile = L"Assets/Animations/CP_Run.anim";
			break;
		}
		break;

	case 2: // Character_Red
		switch (type)
		{
		case CharacterAnimationType::eIdle:
			animFile = L"Assets/Animations/CR_Idle.anim";
			break;

		case CharacterAnimationType::eRun:
			animFile = L"Assets/Animations/CR_Run.anim";
			break;
		}
		break;
	}

	return animFile;
}

void GetEnemyFiles(uint8 enemyType, wstring* outMeshFile, wstring* outTexFile, wstring* outSkelFile)
{
	switch (enemyType)
	{
	case Virus:
		*outMeshFile = L"Assets/Meshes/Virus.mesh";
		*outTexFile = L"Assets/Textures/Virus.png";
		*outSkelFile = L"Assets/Skeletons/Virus.skel";
		break;

	case Dog:
		*outMeshFile = L"Assets/Meshes/Dog.mesh";
		*outTexFile = L"Assets/Textures/Dog.png";
		*outSkelFile = L"Assets/Skeletons/Dog.skel";
		break;

	default:
		break;
	}
}

wstring GetEnemyAnimation(uint8 enemyType, EnemyAnimationType animType)
{
	wstring animFile;

	switch (enemyType)
	{
	case Virus:
		switch (animType)
		{
		case EnemyAnimationType::eIdle:
			animFile = L"Assets/Animations/Virus_Idle.anim";
			break;

		case EnemyAnimationType::eRun:
			animFile = L"Assets/Animations/Virus_Run.anim";
			break;

		case EnemyAnimationType::eAttack:
			animFile = L"Assets/Animations/Virus_Attack.anim";
			break;
		}
		break;

	case Dog:
		switch (animType)
		{
		case EnemyAnimationType::eIdle:
			animFile = L"Assets/Animations/Dog_Idle.anim";
			break;

		case EnemyAnimationType::eRun:
			animFile = L"Assets/Animations/Dog_Run.anim";
			break;

		case EnemyAnimationType::eAttack:
			animFile = L"Assets/Animations/Dog_Attack.anim";
			break;
		}
		break;
	default:
		HB_LOG("Unknown enemy animation");
		break;
	}

	return animFile;
}
