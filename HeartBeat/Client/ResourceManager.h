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
	static Mesh* GetMesh(const wstring& path);
	static Texture* GetTexture(const wstring& path);
	static Skeleton* GetSkeleton(const wstring& path);
	static Animation* GetAnimation(const wstring& path);
	static AABB* GetAABB(const wstring& path);
	static Mesh* GetDebugMesh(const wstring& path);
	static Font* GetFont(const wstring& path);

private:
	static unordered_map<wstring, Mesh*> sMeshes;
	static unordered_map<wstring, Texture*> sTextures;
	static unordered_map<wstring, Skeleton*> sSkeletons;
	static unordered_map<wstring, Animation*> sAnimations;
	static unordered_map<wstring, AABB*> sBoxes;
	static unordered_map<wstring, Mesh*> sDebugMeshes;
	static unordered_map<wstring, Font*> sFonts;
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

const wstring ASSET_PATH = L"../Assets/";

#define MESH(x) GetMeshFile(x)
#define TEXTURE(x) GetTextureFile(x)
#define SKELETON(x) GetSkeletonFile(x)
#define ANIM(x) GetAnimFile(x)
#define BOX(x) GetBoxFile(x)
#define DEBUGMESH(x) GetBoxFile(x)
#define FONT(x) GetFontFile(x)

inline Mesh* GetMeshFile(const wstring& file)
{
	return ResourceManager::GetMesh(ASSET_PATH + L"Meshes/" + file);
}

inline Texture* GetTextureFile(const wstring& file)
{
	return ResourceManager::GetTexture(ASSET_PATH + L"Textures/" + file);
}

inline Skeleton* GetSkeletonFile(const wstring& file)
{
	return ResourceManager::GetSkeleton(ASSET_PATH + L"Skeletons/" + file);
}

inline Animation* GetAnimFile(const wstring& file)
{
	return ResourceManager::GetAnimation(ASSET_PATH + L"Animations/" + file);
}

inline AABB* GetBoxFile(const wstring& file)
{
	return ResourceManager::GetAABB(ASSET_PATH + L"Boxes/" + file);
}

inline Font* GetFontFile(const wstring& file)
{
	return ResourceManager::GetFont(ASSET_PATH + L"Fonts/" + file);
}


