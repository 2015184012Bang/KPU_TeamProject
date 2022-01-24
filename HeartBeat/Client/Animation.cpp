#include "ClientPCH.h"
#include "Animation.h"

#include <rapidjson/document.h>

#include "Skeleton.h"

Animation::Animation()
	: mNumBones(0)
	, mNumFrames(0)
	, mDuration(0.0f)
	, mFrameDuration(0.0f)
{

}

void Animation::Load(const wstring& path)
{
	std::ifstream file(path);

	if (!file.is_open())
	{
		HB_LOG("Could not open file: {0}", ws2s(path));
		HB_ASSERT(false, "ASSERTION FAILED");
	}

	std::stringstream fileStream;
	fileStream << file.rdbuf();
	string contents = fileStream.str();
	rapidjson::StringStream jsonStr(contents.c_str());
	rapidjson::Document doc;
	doc.ParseStream(jsonStr);

	if (!doc.IsObject())
	{
		HB_LOG("{0} is not valid json file!", ws2s(path));
		HB_ASSERT(false, "ASSERTION FAILED");
	}

	const rapidjson::Value& sequence = doc["sequence"];
	const rapidjson::Value& frames = sequence["frames"];
	const rapidjson::Value& length = sequence["length"];
	const rapidjson::Value& bonecount = sequence["bonecount"];

	mNumFrames = frames.GetUint();
	mDuration = length.GetFloat();
	mNumBones = bonecount.GetUint();
	mFrameDuration = mDuration / (mNumFrames - 1);

	mTracks.resize(mNumBones);

	const rapidjson::Value& tracks = sequence["tracks"];

	for (rapidjson::SizeType i = 0; i < tracks.Size(); i++)
	{
		UINT boneIndex = tracks[i]["bone"].GetUint();

		const rapidjson::Value& transforms = tracks[i]["transforms"];

		BoneTransform temp;

		for (rapidjson::SizeType j = 0; j < transforms.Size(); j++)
		{
			const rapidjson::Value& rot = transforms[j]["rot"];
			const rapidjson::Value& trans = transforms[j]["trans"];

			temp.Rotation.x = rot[0].GetFloat();
			temp.Rotation.y = rot[1].GetFloat();
			temp.Rotation.z = rot[2].GetFloat();
			temp.Rotation.w = rot[3].GetFloat();

			temp.Translation.x = trans[0].GetFloat();
			temp.Translation.y = trans[1].GetFloat();
			temp.Translation.z = trans[2].GetFloat();

			mTracks[boneIndex].emplace_back(temp);
		}
	}
}

void Animation::GetGlobalPoseAtTime(vector<Matrix>* outPoses, const Skeleton* skeleton, float t) const
{
	if ((*outPoses).size() != mNumBones)
	{
		(*outPoses).resize(mNumBones);
	}

	UINT frame = static_cast<UINT>(t / mFrameDuration);
	UINT nextFrame = frame + 1;

	float pct = t / mFrameDuration - frame;

	if (mTracks[0].size() > 0)
	{
		BoneTransform interp = BoneTransform::Interpolate(mTracks[0][frame],
			mTracks[0][nextFrame], pct);
		(*outPoses)[0] = interp.ToMatrix();
	}
	else
	{
		(*outPoses)[0] = Matrix::Identity;
	}

	const std::vector<Bone>& bones = skeleton->GetAllBones();
	for (UINT bone = 1; bone < mNumBones; bone++)
	{
		Matrix mat;

		if (mTracks[bone].size() > 0)
		{
			BoneTransform interp = BoneTransform::Interpolate(mTracks[bone][frame],
				mTracks[bone][nextFrame], pct);

			mat = interp.ToMatrix();
		}

		mat *= (*outPoses)[bones[bone].Parent];
		(*outPoses)[bone] = mat;
	}
}
