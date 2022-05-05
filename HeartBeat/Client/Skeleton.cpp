#include "ClientPCH.h"
#include "Skeleton.h"

#include <rapidjson/document.h>

void Skeleton::Load(string_view path)
{
	std::ifstream file(path.data());

	if (!file.is_open())
	{
		HB_ASSERT(false, "Could not open skeleton file");
	}

	std::stringstream fileStream;
	fileStream << file.rdbuf();
	string contents = fileStream.str();
	rapidjson::StringStream jsonStr(contents.c_str());
	rapidjson::Document doc;
	doc.ParseStream(jsonStr);

	if (!doc.IsObject())
	{
		HB_ASSERT(false, "Its not valid json file.");
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

uint32 Skeleton::GetBoneIndexByName(string_view name) const
{
	auto iter = std::find_if(mBones.begin(), mBones.end(), [&name](const Bone& bone) {
		return bone.Name == name;
		});

	if (iter != mBones.end())
	{
		return static_cast<uint32>(iter - mBones.begin());
	}
	else
	{
		HB_ASSERT(false, "There is no bone name: {0}", name);
		return -1;
	}
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
