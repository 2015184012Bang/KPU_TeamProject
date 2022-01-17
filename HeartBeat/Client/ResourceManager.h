#pragma once

#include "Mesh.h"
#include "Texture.h"
#include "Skeleton.h"

class ResourceManager
{
public:
	static void Shutdown();
	static Mesh* GetMesh(const wstring& path);
	static Texture* GetTexture(const wstring& path);
	static Skeleton* GetSkeleton(const wstring& path);

private:
	static unordered_map<wstring, Mesh*> sMeshes;
	static unordered_map<wstring, Texture*> sTextures;
	static unordered_map<wstring, Skeleton*> sSkeletons;
};

