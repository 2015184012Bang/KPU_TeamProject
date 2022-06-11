#pragma once

#include "ClientPCH.h"

constexpr uint32 MAX_SKELETON_BONES = 256;

struct BoneTransform
{
	Vector3 Translation;
	Quaternion Rotation;

	Matrix ToMatrix() const
	{
		Matrix mat = Matrix::CreateFromQuaternion(Rotation);
		mat *= Matrix::CreateTranslation(Translation);

		return mat;
	}

	static BoneTransform Interpolate(const BoneTransform& a, const BoneTransform& b, float f)
	{
		BoneTransform t;
		t.Rotation = Quaternion::Slerp(a.Rotation, b.Rotation, f);
		t.Translation = Vector3::Lerp(a.Translation, b.Translation, f);

		return t;
	}
};

struct Bone
{
	BoneTransform LocalBindPose;
	string Name;
	int Parent;
};

struct MatrixPalette
{
	Matrix Entry[MAX_SKELETON_BONES];
	Matrix CurrentPoses[MAX_SKELETON_BONES];
};