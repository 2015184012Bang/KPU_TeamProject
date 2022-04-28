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

Mesh* ResourceManager::GetMesh(string_view path)
{
	auto iter = sMeshes.find(path.data());

	if (iter != sMeshes.end())
	{
		return iter->second;
	}
	else
	{
		Mesh* newMesh = new Mesh;
		newMesh->Load(path);
		sMeshes[path.data()] = newMesh;

		return newMesh;
	}
}

Texture* ResourceManager::GetTexture(string_view path)
{
	auto iter = sTextures.find(path.data());

	if (iter != sTextures.end())
	{
		return iter->second;
	}
	else
	{
		Texture* newTexture = new Texture;
		newTexture->Load(path);
		sTextures[path.data()] = newTexture;

		return newTexture;
	}
}

Skeleton* ResourceManager::GetSkeleton(string_view path)
{
	auto iter = sSkeletons.find(path.data());
	
	if (iter != sSkeletons.end())
	{
		return iter->second;
	}
	else
	{
		Skeleton* newSkel = new Skeleton;
		newSkel->Load(path);
		sSkeletons[path.data()] = newSkel;

		return newSkel;
	}
}

Animation* ResourceManager::GetAnimation(string_view path)
{
	auto iter = sAnimations.find(path.data());

	if (iter != sAnimations.end())
	{
		return iter->second;
	}
	else
	{
		Animation* newAnim = new Animation;
		newAnim->Load(path);
		sAnimations[path.data()] = newAnim;

		return newAnim;
	}
}

AABB* ResourceManager::GetAABB(string_view path)
{
	auto iter = sBoxes.find(path.data());

	if (iter != sBoxes.end())
	{
		return iter->second;
	}
	else
	{
		AABB* newBox = new AABB;
		newBox->Load(path);
		sBoxes[path.data()] = newBox;

		Mesh* newDebugMesh = new Mesh;
		newDebugMesh->LoadDebugMesh(newBox->GetMin(), newBox->GetMax());
		sDebugMeshes[path.data()] = newDebugMesh;

		return newBox;
	}
}

Mesh* ResourceManager::GetDebugMesh(string_view path)
{
	auto iter = sDebugMeshes.find(path.data());

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

Font* ResourceManager::GetFont(string_view path)
{
	auto iter = sFonts.find(path.data());

	if (iter != sFonts.end())
	{
		return iter->second;
	}
	else
	{
		Font* newFont = new Font;
		newFont->Load(path);
		sFonts[path.data()] = newFont;

		return newFont;
	}
}

void ResourceManager::MakeAnimTransitions()
{
	// 바이러스 애니메이션 트랜지션 설정
	{
		Animation* idleAnim = ANIM("Virus_Idle.anim");
		Animation* runningAnim = ANIM("Virus_Run.anim");
		Animation* attackingAnim = ANIM("Virus_Attack.anim");
		attackingAnim->SetLoop(false);

		idleAnim->AddTransition("Run", runningAnim);
		idleAnim->AddTransition("Attack", attackingAnim);
		runningAnim->AddTransition("Idle", idleAnim);
		attackingAnim->AddTransition("WhenEnd", idleAnim);
	}

	// 캐릭터_그린
	{
		Animation* idleAnim = ANIM("CG_Idle.anim");
		Animation* runningAnim = ANIM("CG_Run.anim");
		Animation* attack1 = ANIM("CG_Attack1.anim");
		Animation* attack2 = ANIM("CG_Attack2.anim");
		Animation* attack3 = ANIM("CG_Attack3.anim");

		// Loop가 false이면 애니메이션이 종료됐을 때 WhenEnd 트리거가 작동한다.
		attack1->SetLoop(false); 
		attack2->SetLoop(false);
		attack3->SetLoop(false);

		idleAnim->AddTransition("Run", runningAnim);
		idleAnim->AddTransition("Attack1", attack1);
		idleAnim->AddTransition("Attack2", attack2);
		idleAnim->AddTransition("Attack3", attack3);

		runningAnim->AddTransition("Idle", idleAnim);
		runningAnim->AddTransition("Attack1", attack1);
		runningAnim->AddTransition("Attack2", attack2);
		runningAnim->AddTransition("Attack3", attack3);

		attack1->AddTransition("WhenEnd", idleAnim);
		attack2->AddTransition("WhenEnd", idleAnim);
		attack3->AddTransition("WhenEnd", idleAnim);
	}

	// 캐릭터_핑크
	{
		Animation* idleAnim = ANIM("CP_Idle.anim");
		Animation* runningAnim = ANIM("CP_Run.anim");
		Animation* attack1 = ANIM("CP_Attack1.anim");
		Animation* attack2 = ANIM("CP_Attack2.anim");
		Animation* attack3 = ANIM("CP_Attack3.anim");

		attack1->SetLoop(false);
		attack2->SetLoop(false);
		attack3->SetLoop(false);

		idleAnim->AddTransition("Run", runningAnim);
		idleAnim->AddTransition("Attack1", attack1);
		idleAnim->AddTransition("Attack2", attack2);
		idleAnim->AddTransition("Attack3", attack3);

		runningAnim->AddTransition("Idle", idleAnim);
		runningAnim->AddTransition("Attack1", attack1);
		runningAnim->AddTransition("Attack2", attack2);
		runningAnim->AddTransition("Attack3", attack3);

		attack1->AddTransition("WhenEnd", idleAnim);
		attack2->AddTransition("WhenEnd", idleAnim);
		attack3->AddTransition("WhenEnd", idleAnim);
	}

	// 캐릭터_레드
	{
		Animation* idleAnim = ANIM("CR_Idle.anim");
		Animation* runningAnim = ANIM("CR_Run.anim");
		Animation* attack1 = ANIM("CR_Attack1.anim");
		Animation* attack2 = ANIM("CR_Attack2.anim");
		Animation* attack3 = ANIM("CR_Attack3.anim");

		attack1->SetLoop(false);
		attack2->SetLoop(false);
		attack3->SetLoop(false);

		idleAnim->AddTransition("Run", runningAnim);
		idleAnim->AddTransition("Attack1", attack1);
		idleAnim->AddTransition("Attack2", attack2);
		idleAnim->AddTransition("Attack3", attack3);

		runningAnim->AddTransition("Idle", idleAnim);
		runningAnim->AddTransition("Attack1", attack1);
		runningAnim->AddTransition("Attack2", attack2);
		runningAnim->AddTransition("Attack3", attack3);

		attack1->AddTransition("WhenEnd", idleAnim);
		attack2->AddTransition("WhenEnd", idleAnim);
		attack3->AddTransition("WhenEnd", idleAnim);
	}

	// NPC(세포)
	{
		Animation* idleAnim = ANIM("Cell_Idle.anim");
		Animation* runningAnim = ANIM("Cell_Run.anim");
		Animation* attackingAnim = ANIM("Cell_Attack.anim");

		idleAnim->AddTransition("Run", runningAnim);
		idleAnim->AddTransition("Attack", attackingAnim);
		runningAnim->AddTransition("Idle", idleAnim);
		runningAnim->AddTransition("Attack", attackingAnim);
	}

	// 탱크
	{
		Animation* idleAnim = ANIM("Tank_Idle.anim");
		Animation* runningAnim = ANIM("Tank_Run.anim");
		idleAnim->AddTransition("Run", runningAnim);
		runningAnim->AddTransition("Idle", idleAnim);
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
