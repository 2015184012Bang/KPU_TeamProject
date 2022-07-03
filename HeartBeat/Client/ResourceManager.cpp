#include "ClientPCH.h"
#include "ResourceManager.h"

#include "AABB.h"
#include "Animation.h"
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

void ResourceManager::Shutdown()
{
	for (auto& [_, mesh] : sMeshes)
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

void ResourceManager::MakeAnimTransitions()
{
	// 바이러스
	{
		Animation* idleAnim = ANIM("Virus_Idle.anim");
		Animation* runningAnim = ANIM("Virus_Run.anim");
		Animation* attackingAnim = ANIM("Virus_Attack.anim");
		attackingAnim->SetLoop(false);

		idleAnim->AddTransition("Run", runningAnim);
		idleAnim->AddTransition("Attack", attackingAnim);
		runningAnim->AddTransition("Idle", idleAnim);
		runningAnim->AddTransition("Attack", attackingAnim);
		attackingAnim->AddTransition("WhenEnd", idleAnim);
	}

	// 개
	{
		Animation* idleAnim = ANIM("Dog_Idle.anim");
		Animation* runningAnim = ANIM("Dog_Run.anim");
		Animation* attackingAnim = ANIM("Dog_Attack.anim");
		Animation* deadAnim = ANIM("Dog_Dead.anim");
		attackingAnim->SetLoop(false);
		deadAnim->SetLoop(false);

		idleAnim->AddTransition("Run", runningAnim);
		idleAnim->AddTransition("Attack", attackingAnim);
		runningAnim->AddTransition("Idle", idleAnim);
		runningAnim->AddTransition("Attack", attackingAnim);
		attackingAnim->AddTransition("WhenEnd", idleAnim);
	}

	// 벽
	{
		Animation* breakAnim = ANIM("Wall_Break.anim");
		breakAnim->SetLoop(false);
	}

	// 폭탄 이펙트
	{
		Animation* explodeAnim = ANIM("Bomb_Explode.anim");
		explodeAnim->SetLoop(false);
	}

	// 캐릭터_그린
	{
		Animation* idleAnim = ANIM("CG_Idle.anim");
		Animation* runningAnim = ANIM("CG_Run.anim");
		Animation* idleNoneAnim = ANIM("CG_Idle_None.anim");
		Animation* runningNoneAnim = ANIM("CG_Run_None.anim");
		Animation* deadAnim = ANIM("CG_Dead.anim");
		Animation* attack1 = ANIM("CG_Attack1.anim");
		Animation* attack2 = ANIM("CG_Attack2.anim");
		Animation* attack3 = ANIM("CG_Attack3.anim");
		Animation* skill1 = ANIM("CG_Skill1.anim");
		Animation* skill2 = ANIM("CG_Skill2.anim");
		Animation* skill3 = ANIM("CG_Skill3.anim");

		// Loop가 false이면 애니메이션이 종료됐을 때 WhenEnd 트리거가 작동한다.
		deadAnim->SetLoop(false);
		attack1->SetLoop(false);
		attack2->SetLoop(false);
		attack3->SetLoop(false);
		skill1->SetLoop(false);
		skill2->SetLoop(false);
		skill3->SetLoop(false);

		idleAnim->AddTransition("Run", runningAnim);
		idleAnim->AddTransition("Attack1", attack1);
		idleAnim->AddTransition("Attack2", attack2);
		idleAnim->AddTransition("Attack3", attack3);
		idleAnim->AddTransition("Skill1", skill1);
		idleAnim->AddTransition("Skill2", skill2);
		idleAnim->AddTransition("Skill3", skill3);

		runningAnim->AddTransition("Idle", idleAnim);
		runningAnim->AddTransition("Attack1", attack1);
		runningAnim->AddTransition("Attack2", attack2);
		runningAnim->AddTransition("Attack3", attack3);
		runningAnim->AddTransition("Skill1", skill1);
		runningAnim->AddTransition("Skill2", skill2);
		runningAnim->AddTransition("Skill3", skill3);

		idleNoneAnim->AddTransition("Run", runningNoneAnim);
		runningNoneAnim->AddTransition("Idle", idleNoneAnim);

		attack1->AddTransition("WhenEnd", idleAnim);
		attack1->AddTransition("Skill1", skill1);
		attack1->AddTransition("Skill2", skill2);
		attack1->AddTransition("Skill3", skill3);
		attack2->AddTransition("WhenEnd", idleAnim);
		attack2->AddTransition("Skill1", skill1);
		attack2->AddTransition("Skill2", skill2);
		attack2->AddTransition("Skill3", skill3);
		attack3->AddTransition("WhenEnd", idleAnim);
		attack3->AddTransition("Skill1", skill1);
		attack3->AddTransition("Skill2", skill2);
		attack3->AddTransition("Skill3", skill3);
		skill1->AddTransition("WhenEnd", idleAnim);
		skill1->AddTransition("Attack1", attack1);
		skill1->AddTransition("Attack2", attack2);
		skill1->AddTransition("Attack3", attack3);
		skill2->AddTransition("WhenEnd", idleAnim);
		skill2->AddTransition("Attack1", attack1);
		skill2->AddTransition("Attack2", attack2);
		skill2->AddTransition("Attack3", attack3);
		skill3->AddTransition("WhenEnd", idleAnim);
		skill3->AddTransition("Attack1", attack1);
		skill3->AddTransition("Attack2", attack2);
		skill3->AddTransition("Attack3", attack3);
	}

	// 캐릭터_핑크
	{
		Animation* idleAnim = ANIM("CP_Idle.anim");
		Animation* runningAnim = ANIM("CP_Run.anim");
		Animation* idleNoneAnim = ANIM("CP_Idle_None.anim");
		Animation* runningNoneAnim = ANIM("CP_Run_None.anim");
		Animation* deadAnim = ANIM("CP_Dead.anim");
		Animation* attack1 = ANIM("CP_Attack1.anim");
		Animation* attack2 = ANIM("CP_Attack2.anim");
		Animation* attack3 = ANIM("CP_Attack3.anim");
		Animation* skill1 = ANIM("CP_Skill1.anim");
		Animation* skill2 = ANIM("CP_Skill2.anim");
		Animation* skill3 = ANIM("CP_Skill3.anim");

		deadAnim->SetLoop(false);
		attack1->SetLoop(false);
		attack2->SetLoop(false);
		attack3->SetLoop(false);
		skill1->SetLoop(false);
		skill2->SetLoop(false);
		skill3->SetLoop(false);

		idleAnim->AddTransition("Run", runningAnim);
		idleAnim->AddTransition("Attack1", attack1);
		idleAnim->AddTransition("Attack2", attack2);
		idleAnim->AddTransition("Attack3", attack3);
		idleAnim->AddTransition("Skill1", skill1);
		idleAnim->AddTransition("Skill2", skill2);
		idleAnim->AddTransition("Skill3", skill3);

		runningAnim->AddTransition("Idle", idleAnim);
		runningAnim->AddTransition("Attack1", attack1);
		runningAnim->AddTransition("Attack2", attack2);
		runningAnim->AddTransition("Attack3", attack3);
		runningAnim->AddTransition("Skill1", skill1);
		runningAnim->AddTransition("Skill2", skill2);
		runningAnim->AddTransition("Skill3", skill3);

		idleNoneAnim->AddTransition("Run", runningNoneAnim);
		runningNoneAnim->AddTransition("Idle", idleNoneAnim);

		attack1->AddTransition("WhenEnd", idleAnim);
		attack1->AddTransition("Skill1", skill1);
		attack1->AddTransition("Skill2", skill2);
		attack1->AddTransition("Skill3", skill3);
		attack2->AddTransition("WhenEnd", idleAnim);
		attack2->AddTransition("Skill1", skill1);
		attack2->AddTransition("Skill2", skill2);
		attack2->AddTransition("Skill3", skill3);
		attack3->AddTransition("WhenEnd", idleAnim);
		attack3->AddTransition("Skill1", skill1);
		attack3->AddTransition("Skill2", skill2);
		attack3->AddTransition("Skill3", skill3);
		skill1->AddTransition("WhenEnd", idleAnim);
		skill1->AddTransition("Attack1", attack1);
		skill1->AddTransition("Attack2", attack2);
		skill1->AddTransition("Attack3", attack3);
		skill2->AddTransition("WhenEnd", idleAnim);
		skill2->AddTransition("Attack1", attack1);
		skill2->AddTransition("Attack2", attack2);
		skill2->AddTransition("Attack3", attack3);
		skill3->AddTransition("WhenEnd", idleAnim);
		skill3->AddTransition("Attack1", attack1);
		skill3->AddTransition("Attack2", attack2);
		skill3->AddTransition("Attack3", attack3);
	}

	// 캐릭터_레드
	{
		Animation* idleAnim = ANIM("CR_Idle.anim");
		Animation* runningAnim = ANIM("CR_Run.anim");
		Animation* idleNoneAnim = ANIM("CR_Idle_None.anim");
		Animation* runningNoneAnim = ANIM("CR_Run_None.anim");
		Animation* deadAnim = ANIM("CR_Dead.anim");
		Animation* attack1 = ANIM("CR_Attack1.anim");
		Animation* attack2 = ANIM("CR_Attack2.anim");
		Animation* attack3 = ANIM("CR_Attack3.anim");
		Animation* skill1 = ANIM("CR_Skill1.anim");
		Animation* skill2 = ANIM("CR_Skill2.anim");
		Animation* skill3 = ANIM("CR_Skill3.anim");

		deadAnim->SetLoop(false);
		attack1->SetLoop(false);
		attack2->SetLoop(false);
		attack3->SetLoop(false);
		skill1->SetLoop(false);
		skill2->SetLoop(false);
		skill3->SetLoop(false);

		idleAnim->AddTransition("Run", runningAnim);
		idleAnim->AddTransition("Attack1", attack1);
		idleAnim->AddTransition("Attack2", attack2);
		idleAnim->AddTransition("Attack3", attack3);
		idleAnim->AddTransition("Skill1", skill1);
		idleAnim->AddTransition("Skill2", skill2);
		idleAnim->AddTransition("Skill3", skill3);

		runningAnim->AddTransition("Idle", idleAnim);
		runningAnim->AddTransition("Attack1", attack1);
		runningAnim->AddTransition("Attack2", attack2);
		runningAnim->AddTransition("Attack3", attack3);
		runningAnim->AddTransition("Skill1", skill1);
		runningAnim->AddTransition("Skill2", skill2);
		runningAnim->AddTransition("Skill3", skill3);

		idleNoneAnim->AddTransition("Run", runningNoneAnim);
		runningNoneAnim->AddTransition("Idle", idleNoneAnim);

		attack1->AddTransition("WhenEnd", idleAnim);
		attack1->AddTransition("Skill1", skill1);
		attack1->AddTransition("Skill2", skill2);
		attack1->AddTransition("Skill3", skill3);
		attack2->AddTransition("WhenEnd", idleAnim);
		attack2->AddTransition("Skill1", skill1);
		attack2->AddTransition("Skill2", skill2);
		attack2->AddTransition("Skill3", skill3);
		attack3->AddTransition("WhenEnd", idleAnim);
		attack3->AddTransition("Skill1", skill1);
		attack3->AddTransition("Skill2", skill2);
		attack3->AddTransition("Skill3", skill3);
		skill1->AddTransition("WhenEnd", idleAnim);
		skill1->AddTransition("Attack1", attack1);
		skill1->AddTransition("Attack2", attack2);
		skill1->AddTransition("Attack3", attack3);
		skill2->AddTransition("WhenEnd", idleAnim);
		skill2->AddTransition("Attack1", attack1);
		skill2->AddTransition("Attack2", attack2);
		skill2->AddTransition("Attack3", attack3);
		skill3->AddTransition("WhenEnd", idleAnim);
		skill3->AddTransition("Attack1", attack1);
		skill3->AddTransition("Attack2", attack2);
		skill3->AddTransition("Attack3", attack3);
	}

	// NPC(세포)
	{
		Animation* idleAnim = ANIM("Cell_Idle.anim");
		Animation* runningAnim = ANIM("Cell_Run.anim");
		Animation* attackingAnim = ANIM("Cell_Attack.anim");
		Animation* dieAnim = ANIM("Cell_Dead.anim");
		attackingAnim->SetLoop(false);
		dieAnim->SetLoop(false);

		idleAnim->AddTransition("Run", runningAnim);
		idleAnim->AddTransition("Attack", attackingAnim);
		runningAnim->AddTransition("Idle", idleAnim);
		runningAnim->AddTransition("Attack", attackingAnim);
		attackingAnim->AddTransition("WhenEnd", idleAnim);
	}

	// 탱크
	{
		Animation* idleAnim = ANIM("Tank_Idle.anim");
		Animation* runningAnim = ANIM("Tank_Run.anim");
		Animation* attackAnim = ANIM("Tank_Attack.anim");
		attackAnim->SetLoop(false);
		idleAnim->AddTransition("Run", runningAnim);
		runningAnim->AddTransition("Idle", idleAnim);
		runningAnim->AddTransition("Attack", attackAnim);
		attackAnim->AddTransition("WhenEnd", runningAnim);
	}



	// 카트
	{
		ANIM("Cart_Run.anim");
	}

	// 문
	{
		Animation* openAnim = ANIM("Door_Open.anim");
		openAnim->SetLoop(false);
	}

	// 보스
	{
		Animation* idleAnim = ANIM("Boss_Idle.anim");
		Animation* startAnim = ANIM("Boss_Start.anim");
		startAnim->SetLoop(false);
		startAnim->AddTransition("WhenEnd", idleAnim);
	}

	// 이펙트
	{
		Animation* fatAnim = ANIM("Fat_Break.anim");
		fatAnim->SetLoop(false);

		Animation* bwallAnim = ANIM("BWall_Break.anim");
		bwallAnim->SetLoop(false);
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
		case CharacterAnimationType::IDLE:
			return ANIM("CG_Idle.anim");

		case CharacterAnimationType::IDLE_NONE:
			return ANIM("CG_Idle_None.anim");

		case CharacterAnimationType::RUN:
			return ANIM("CG_Run.anim");

		case CharacterAnimationType::RUN_NONE:
			return ANIM("CG_Run_None.anim");

		case CharacterAnimationType::DEAD:
			return ANIM("CG_Dead.anim");
		}
		break;

	case 1: // Character_Pink
		switch (type)
		{
		case CharacterAnimationType::IDLE:
			return ANIM("CP_Idle.anim");

		case CharacterAnimationType::RUN:
			return ANIM("CP_Run.anim");

		case CharacterAnimationType::IDLE_NONE:
			return ANIM("CP_Idle_None.anim");

		case CharacterAnimationType::RUN_NONE:
			return ANIM("CP_Run_None.anim");

		case CharacterAnimationType::DEAD:
			return ANIM("CP_Dead.anim");
		}
		break;

	case 2: // Character_Red
		switch (type)
		{
		case CharacterAnimationType::IDLE:
			return ANIM("CR_Idle.anim");

		case CharacterAnimationType::RUN:
			return ANIM("CR_Run.anim");

		case CharacterAnimationType::IDLE_NONE:
			return ANIM("CR_Idle_None.anim");

		case CharacterAnimationType::RUN_NONE:
			return ANIM("CR_Run_None.anim");

		case CharacterAnimationType::DEAD:
			return ANIM("CR_Dead.anim");
		}
		break;
	}

	return nullptr;
}

std::tuple<Mesh*, Texture*, Skeleton*> GetEnemyFiles(EnemyType enemyType)
{
	switch (enemyType)
	{
	case EnemyType::VIRUS:
		return { MESH("Virus.mesh") , TEXTURE("Virus.png") , SKELETON("Virus.skel") };

	case EnemyType::DOG:
		return { MESH("Dog.mesh") , TEXTURE("Dog.png") , SKELETON("Dog.skel") };

	default:
		HB_LOG("UnKnown Enemy Type!");
		return { nullptr, nullptr, nullptr };
	}
}

Animation* GetEnemyAnimation(EnemyType enemyType, EnemyAnimationType animType)
{
	switch (enemyType)
	{
	case EnemyType::VIRUS:
		switch (animType)
		{
		case EnemyAnimationType::IDLE:
			return ANIM("Virus_Idle.anim");

		case EnemyAnimationType::RUN:
			return ANIM("Virus_Run.anim");

		case EnemyAnimationType::ATTACK:
			return ANIM("Virus_Attack.anim");

		case EnemyAnimationType::DEAD:
			return ANIM("Virus_Dead.anim");
		}
		break;

	case EnemyType::DOG:
		switch (animType)
		{
		case EnemyAnimationType::IDLE:
			return ANIM("Dog_Idle.anim");

		case EnemyAnimationType::RUN:
			return ANIM("Dog_Run.anim");

		case EnemyAnimationType::ATTACK:
			return ANIM("Dog_Attack.anim");
		}
		break;
	default:
		HB_LOG("Unknown enemy animation");
		break;
	}

	return nullptr;
}
