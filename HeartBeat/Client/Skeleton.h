#pragma once

#include "Resource.h"
#include "Bone.h"

class Skeleton : public IResource
{
public:
	Skeleton() = default;

	virtual void Load(const wstring& path) override;

	uint32 GetNumBones() const { return static_cast<uint32>(mBones.size()); }
	const Bone& GetBone(uint32 idx) const { return mBones[idx]; }
	const vector<Bone>& GetAllBones() const { return mBones; }
	const vector<Matrix>& GetGlobalInvBindPoses() const { return mGlobalInvBindPoses; }

private:
	void computeGlobalInvBindPose();

private:
	vector<Bone> mBones;
	vector<Matrix> mGlobalInvBindPoses;
};

