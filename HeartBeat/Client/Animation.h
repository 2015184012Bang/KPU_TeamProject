#pragma once

#include "Resource.h"
#include "Bone.h"

class Skeleton;

class Animation : public IResource
{
public:
	Animation();

	virtual void Load(const wstring& path) override;
	
	uint32 GetNumBones() const { return mNumBones; }
	uint32 GetNumFrames() const { return mNumFrames; }
	float GetDuration() const { return mDuration; }
	float GetFrameDuration() const { return mFrameDuration; }

	void GetGlobalPoseAtTime(vector<Matrix>* outPoses, const Skeleton* skeleton, float t) const;

private:
	uint32 mNumBones;
	uint32 mNumFrames;
	float mDuration;
	float mFrameDuration;

	vector<vector<BoneTransform>> mTracks;
};

