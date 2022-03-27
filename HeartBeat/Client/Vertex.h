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
	SpriteVertex()
		: Position(Vector3::Zero)
		, UV(Vector2::Zero) {}

	SpriteVertex(float x, float y, float z, float u, float v)
		: Position(x, y, z)
		, UV(u, v) {}

	Vector3 Position;
	Vector2 UV;
};