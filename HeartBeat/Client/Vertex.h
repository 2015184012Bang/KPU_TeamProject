#pragma once

#include "ClientPCH.h"

struct Vertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector2 UV;
};

struct SkeletalVertex
{
	Vector3 Position;
	Vector3 Normal;
	uint32 Bone[4];
	Vector4 BoneWeight;
	Vector2 UV;
};

struct SpriteVertex
{
	Vector3 Position;
	Vector2 UV;
};