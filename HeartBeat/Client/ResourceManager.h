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
	static Mesh* GetMesh(const string& path);
	static Texture* GetTexture(const string& path);
	static Skeleton* GetSkeleton(const string& path);
	static Animation* GetAnimation(const string& path);
	static AABB* GetAABB(const string& path);
	static Mesh* GetDebugMesh(const string& path);
	static Font* GetFont(const string& path);

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
	eRun,
};

enum class EnemyAnimationType
{
	eIdle,
	eRun,
	eAttack,
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

inline Mesh* GetMeshFile(const string& file)
{
	return ResourceManager::GetMesh(ASSET_PATH + "Meshes/" + file);
}

inline Texture* GetTextureFile(const string& file)
{
	return ResourceManager::GetTexture(ASSET_PATH + "Textures/" + file);
}

inline Skeleton* GetSkeletonFile(const string& file)
{
	return ResourceManager::GetSkeleton(ASSET_PATH + "Skeletons/" + file);
}

inline Animation* GetAnimFile(const string& file)
{
	return ResourceManager::GetAnimation(ASSET_PATH + "Animations/" + file);
}

inline AABB* GetBoxFile(const string& file)
{
	return ResourceManager::GetAABB(ASSET_PATH + "Boxes/" + file);
}

inline Font* GetFontFile(const string& file)
{
	return ResourceManager::GetFont(ASSET_PATH + "Fonts/" + file);
}


