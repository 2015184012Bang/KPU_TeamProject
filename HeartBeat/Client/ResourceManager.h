#pragma once

#include "Mesh.h"
#include "Texture.h"

class ResourceManager
{
public:
	static void Shutdown();
	static Mesh* GetMesh(const wstring& path);
	static Texture* GetTexture(const wstring& path);

private:
	static unordered_map<wstring, Mesh*> mMeshes;
	static unordered_map<wstring, Texture*> mTextures;
};

