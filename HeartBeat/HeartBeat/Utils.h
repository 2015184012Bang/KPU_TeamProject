#pragma once

#include <Windows.h>
#include <string>

using std::wstring;
using std::string;

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

inline wstring s2ws(const string& s)
{
	unsigned int len;
	unsigned int slength = static_cast<unsigned int>(s.length()) + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	wstring ret(buf);
	delete[] buf;
	return ret;
}

inline string ws2s(const wstring& s)
{
	unsigned int len;
	unsigned int slength = static_cast<unsigned int>(s.length()) + 1;
	len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
	string r(len, '\0');
	WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0);
	return r;
}