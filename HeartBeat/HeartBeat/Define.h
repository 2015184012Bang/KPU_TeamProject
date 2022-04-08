#pragma once

#include <string>

using std::wstring;

constexpr int MAX_PLAYER = 1;

enum : uint8
{
	Virus,
	Dog,
};

const wstring ASSET_PATH = L"../Assets/";

#define MESH(x) GetMeshPath(x)
#define TEXTURE(x) GetTexturePath(x)
#define SKELETON(x) GetSkeletonPath(x)
#define ANIM(x) GetAnimPath(x)
#define BOX(x) GetBoxPath(x)
#define DEBUGMESH(x) GetBoxPath(x)
#define FONT(x) GetFontPath(x)

static wstring GetMeshPath(const wstring& file)
{
	return ASSET_PATH + L"Meshes/" + file;
}

static wstring GetTexturePath(const wstring& file)
{
	return ASSET_PATH + L"Textures/" + file;
}

static wstring GetSkeletonPath(const wstring& file)
{
	return ASSET_PATH + L"Skeletons/" + file;
}

static wstring GetAnimPath(const wstring& file)
{
	return ASSET_PATH + L"Animations/" + file;
}

static wstring GetBoxPath(const wstring& file)
{
	return ASSET_PATH + L"Boxes/" + file;
}

static wstring GetFontPath(const wstring& file)
{
	return ASSET_PATH + L"Fonts/" + file;
}