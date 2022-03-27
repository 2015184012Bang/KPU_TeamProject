#include "ClientPCH.h"
#include "Skeleton.h"

#include <rapidjson/document.h>

void Skeleton::Load(const wstring& path)
{
	std::ifstream file(path);

	if (!file.is_open())
	{
		HB_LOG("Could not open skeleton file: {0}", ws2s(path));
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
	}

	const rapidjson::Value& boneCount = doc["bonecount"];
	uint32 count = boneCount.GetUint();

	if (count > MAX_SKELETON_BONES)
	{
		HB_ASSERT(false, "Bones exceeds maximum bone count.");
	}

	mBones.reserve(count);
	const rapidjson::Value& bones = doc["bones"];

	Bone temp;
	for (rapidjson::SizeType i = 0; i < count; ++i)
	{
		const rapidjson::Value& name = bones[i]["name"];
		temp.Name = name.GetString();

		const rapidjson::Value& parent = bones[i]["parent"];
		temp.Parent = parent.GetInt();

		const rapidjson::Value& bindPose = bones[i]["bindpose"];

		const rapidjson::Value& rot = bindPose["rot"];
		const rapidjson::Value& trans = bindPose["trans"];

		temp.LocalBindPose.Rotation.x = rot[0].GetFloat();
		temp.LocalBindPose.Rotation.y = rot[1].GetFloat();
		temp.LocalBindPose.Rotation.z = rot[2].GetFloat();
		temp.LocalBindPose.Rotation.w = rot[3].GetFloat();

		temp.LocalBindPose.Translation.x = trans[0].GetFloat();
		temp.LocalBindPose.Translation.y = trans[1].GetFloat();
		temp.LocalBindPose.Translation.z = trans[2].GetFloat();

		mBones.push_back(temp);
	}

	computeGlobalInvBindPose();
}

void Skeleton::computeGlobalInvBindPose()
{
	mGlobalInvBindPoses.resize(GetNumBones());

	mGlobalInvBindPoses[0] = mBones[0].LocalBindPose.ToMatrix();

	for (uint32 i = 1; i < mGlobalInvBindPoses.size(); i++)
	{
		Matrix mat = mBones[i].LocalBindPose.ToMatrix();
		mat *= mGlobalInvBindPoses[mBones[i].Parent];

		mGlobalInvBindPoses[i] = mat;
	}

	for (uint32 i = 0; i < mGlobalInvBindPoses.size(); i++)
	{
		mGlobalInvBindPoses[i] = mGlobalInvBindPoses[i].Invert();
	}
}
