#pragma once

#include "Mesh.h"
#include "Texture.h"
#include "Skeleton.h"
#include "Animation.h"
#include "AABB.h"
#include "Font.h"

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

