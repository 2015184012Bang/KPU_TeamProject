#pragma once

#include "Define.h"

class Mesh;
class Texture;
class Skeleton;
class Animation;
class AABB;
class Font;

class ResourceManager
{
public:
	static void Shutdown();
	static Mesh* GetMesh(string_view path);
	static Texture* GetTexture(string_view path);
	static Skeleton* GetSkeleton(string_view path);
	static Animation* GetAnimation(string_view path);
	static AABB* GetAABB(string_view path);
	static Mesh* GetDebugMesh(string_view path);
	static Font* GetFont(string_view path);

	static void MakeAnimTransitions();

private:
	static unordered_map<string, Mesh*> sMeshes;
	static unordered_map<string, Texture*> sTextures;
	static unordered_map<string, Skeleton*> sSkeletons;
	static unordered_map<string, Animation*> sAnimations;
	static unordered_map<string, AABB*> sBoxes;
	static unordered_map<string, Mesh*> sDebugMeshes;
	static unordered_map<string, Font*> sFonts;
};

enum class CharacterAnimationType
{
	eIdle,
	eIdleNone,
	eRun,
	eRunNone,
};

enum class EnemyAnimationType
{
	eIdle,
	eRun,
	eAttack,
	eDead,
};

std::tuple<Mesh*, Texture*, Skeleton*> GetCharacterFiles(int clientID);
Animation* GetCharacterAnimationFile(int clientID, CharacterAnimationType type);
std::tuple<Mesh*, Texture*, Skeleton*> GetEnemyFiles(eEnemyType enemyType);
Animation* GetEnemyAnimation(eEnemyType enemyType, EnemyAnimationType animType);

const string ASSET_PATH = "../Assets/";

#define MESH(x) GetMeshFile(x)
#define TEXTURE(x) GetTextureFile(x)
#define SKELETON(x) GetSkeletonFile(x)
#define ANIM(x) GetAnimFile(x)
#define BOX(x) GetBoxFile(x)
#define DEBUGMESH(x) GetBoxFile(x)
#define FONT(x) GetFontFile(x)

inline Mesh* GetMeshFile(string_view file)
{
	return ResourceManager::GetMesh(ASSET_PATH + "Meshes/" + file.data());
}

inline Texture* GetTextureFile(string_view file)
{
	return ResourceManager::GetTexture(ASSET_PATH + "Textures/" + file.data());
}

inline Skeleton* GetSkeletonFile(string_view file)
{
	return ResourceManager::GetSkeleton(ASSET_PATH + "Skeletons/" + file.data());
}

inline Animation* GetAnimFile(string_view file)
{
	return ResourceManager::GetAnimation(ASSET_PATH + "Animations/" + file.data());
}

inline AABB* GetBoxFile(string_view file)
{
	return ResourceManager::GetAABB(ASSET_PATH + "Boxes/" + file.data());
}

inline Font* GetFontFile(string_view file)
{
	return ResourceManager::GetFont(ASSET_PATH + "Fonts/" + file.data());
}


