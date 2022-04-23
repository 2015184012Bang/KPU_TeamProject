#pragma once

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

void GetCharacterFiles(int clientID, wstring* outMeshFile, wstring* outTexFile, wstring* outSkelFile);
wstring GetCharacterAnimationFile(int clientID, CharacterAnimationType type);
void GetEnemyFiles(uint8 enemyType, wstring* outMeshFile, wstring* outTexFile, wstring* outSkelFile);
wstring GetEnemyAnimation(uint8 enemyType, EnemyAnimationType animType);


