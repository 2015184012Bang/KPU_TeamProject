#pragma once

#include "Resource.h"
#include "Bone.h"

class Skeleton;

constexpr float ANIM_BLEND_TIME = 0.2f;

class Animation : public IResource
{
public:
	Animation();

	virtual void Load(string_view path) override;
	
	uint32 GetNumBones() const { return mNumBones; }
	uint32 GetNumFrames() const { return mNumFrames; }
	float GetDuration() const { return mDuration; }
	float GetFrameDuration() const { return mFrameDuration; }

	void GetGlobalPoseAtTime(vector<Matrix>* outPoses, const Skeleton* skeleton, float t) const;

	void SetLoop(bool value) { mbLoop = value; }
	bool IsLoop() const { return mbLoop; }

	Animation* FindNextAnimation(string_view triggerName) const;

	void AddTransition(string_view triggerName, Animation* anim);
	void RemoveTransition(string_view triggerName);

private:
	uint32 mNumBones;
	uint32 mNumFrames;
	float mDuration;
	float mFrameDuration;
	bool mbLoop;

	vector<vector<BoneTransform>> mTracks;
	unordered_map<string, Animation*> mTransitions;
};

