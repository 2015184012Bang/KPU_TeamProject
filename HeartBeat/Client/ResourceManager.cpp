#include "ClientPCH.h"
#include "ResourceManager.h"

#include "AABB.h"
#include "Animation.h"
#include "Font.h"
#include "Mesh.h"
#include "Skeleton.h"
#include "Texture.h"
#include "Utils.h"

unordered_map<string, Mesh*> ResourceManager::sMeshes;
unordered_map<string, Texture*> ResourceManager::sTextures;
unordered_map<string, Skeleton*> ResourceManager::sSkeletons;
unordered_map<string, Animation*> ResourceManager::sAnimations;
unordered_map<string, AABB*> ResourceManager::sBoxes;
unordered_map<string, Mesh*> ResourceManager::sDebugMeshes;
unordered_map<string, Font*> ResourceManager::sFonts;

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

Mesh* ResourceManager::GetMesh(const string& path)
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

Texture* ResourceManager::GetTexture(const string& path)
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

Skeleton* ResourceManager::GetSkeleton(const string& path)
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

Animation* ResourceManager::GetAnimation(const string& path)
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

AABB* ResourceManager::GetAABB(const string& path)
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

Mesh* ResourceManager::GetDebugMesh(const string& path)
{
	auto iter = sDebugMeshes.find(path);

	if (iter != sDebugMeshes.end())
	{
		return iter->second;
	}
	else
	{
		HB_ASSERT(false, "Debug mesh file not found");
	}

	return nullptr;
}

Font* ResourceManager::GetFont(const string& path)
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

std::tuple<Mesh*, Texture*, Skeleton*> GetCharacterFiles(int clientID)
{
	switch (clientID)
	{
	case 0:
		return { MESH("Character_Green.mesh"), TEXTURE("Character_Green.png"), SKELETON("Character_Green.skel") };

	case 1:
		return { MESH("Character_Pink.mesh"), TEXTURE("Character_Pink.png"), SKELETON("Character_Pink.skel") };

	case 2:
		return { MESH("Character_Red.mesh"), TEXTURE("Character_Red.png"), SKELETON("Character_Red.skel") };

	default:
		HB_LOG("Unknown client id: {0}", clientID);
		return { nullptr, nullptr, nullptr };
	}
}

Animation* GetCharacterAnimationFile(int clientID, CharacterAnimationType type)
{
	switch (clientID)
	{
	case 0: // Character_Green
		switch (type)
		{
		case CharacterAnimationType::eIdle:
			return ANIM("CG_Idle.anim");
			break;

		case CharacterAnimationType::eRun:
			return ANIM("CG_Run.anim");
			break;
		}
		break;

	case 1: // Character_Pink
		switch (type)
		{
		case CharacterAnimationType::eIdle:
			return ANIM("CP_Idle.anim");
			break;

		case CharacterAnimationType::eRun:
			return ANIM("CP_Run.anim");
			break;
		}
		break;

	case 2: // Character_Red
		switch (type)
		{
		case CharacterAnimationType::eIdle:
			return ANIM("CR_Idle.anim");
			break;

		case CharacterAnimationType::eRun:
			return ANIM("CR_Run.anim");
			break;
		}
		break;
	}

	return nullptr;
}

std::tuple<Mesh*, Texture*, Skeleton*> GetEnemyFiles(eEnemyType enemyType)
{
	switch (enemyType)
	{
	case eEnemyType::Virus:
		return { MESH("Virus.mesh") , TEXTURE("Virus.png") , SKELETON("Virus.skel") };

	case eEnemyType::Dog:
		return { MESH("Dog.mesh") , TEXTURE("Dog.png") , SKELETON("Dog.skel") };

	default:
		HB_LOG("UnKnown Enemy Type!");
		return { nullptr, nullptr, nullptr };
	}
}

Animation* GetEnemyAnimation(eEnemyType enemyType, EnemyAnimationType animType)
{
	switch (enemyType)
	{
	case eEnemyType::Virus:
		switch (animType)
		{
		case EnemyAnimationType::eIdle:
			return ANIM("Virus_Idle.anim");
			break;

		case EnemyAnimationType::eRun:
			return ANIM("Virus_Run.anim");
			break;

		case EnemyAnimationType::eAttack:
			return ANIM("Virus_Attack.anim");
			break;
		}
		break;

	case eEnemyType::Dog:
		switch (animType)
		{
		case EnemyAnimationType::eIdle:
			return ANIM("Dog_Idle.anim");
			break;

		case EnemyAnimationType::eRun:
			return ANIM("Dog_Run.anim");
			break;

		case EnemyAnimationType::eAttack:
			return ANIM("Dog_Attack.anim");
			break;
		}
		break;
	default:
		HB_LOG("Unknown enemy animation");
		break;
	}

	return nullptr;
}
