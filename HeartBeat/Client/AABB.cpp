#include "ClientPCH.h"
#include "AABB.h"

#include <rapidjson/document.h>

#include "Utils.h"

AABB::AABB()
	: mMin(FLT_MAX, FLT_MAX, FLT_MAX)
	, mMax(FLT_MIN, FLT_MIN, FLT_MIN)
{

}

AABB::AABB(const string& path)
	: mMin(FLT_MAX, FLT_MAX, FLT_MAX)
	, mMax(FLT_MIN, FLT_MIN, FLT_MIN)
{
	Load(path);
}

void AABB::Load(const string& path)
{
	std::ifstream file(path);

	if (!file.is_open())
	{
		HB_ASSERT(false, "Could not open file");
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

	const rapidjson::Value& vertsJson = doc["vertices"];

	for (rapidjson::SizeType i = 0; i < vertsJson.Size(); ++i)
	{
		const rapidjson::Value& vert = vertsJson[i];

		if (!vert.IsArray())
		{
			HB_ASSERT(false, "Invalid vertex format");
		}

		Vector3 point;
		point.x = vert[0].GetFloat();
		point.y = vert[1].GetFloat();
		point.z = vert[2].GetFloat();

		updateMinMax(point);
	}
}

void AABB::updateMinMax(const Vector3& point)
{
	mMin.x = std::min(mMin.x, point.x);
	mMin.y = std::min(mMin.y, point.y);
	mMin.z = std::min(mMin.z, point.z);
	mMax.x = std::max(mMax.x, point.x);
	mMax.y = std::max(mMax.y, point.y);
	mMax.z = std::max(mMax.z, point.z);
}

void AABB::rotateY(float yaw)
{
	array<Vector3, 8> points;

	points[0] = mMin;
	points[1] = Vector3(mMax.x, mMin.y, mMin.z);
	points[2] = Vector3(mMin.x, mMax.y, mMin.z);
	points[3] = Vector3(mMin.x, mMin.y, mMax.z);
	points[4] = Vector3(mMin.x, mMax.y, mMax.z);
	points[5] = Vector3(mMax.x, mMin.y, mMax.z);
	points[6] = Vector3(mMax.x, mMax.y, mMin.z);
	points[7] = Vector3(mMax);

	Quaternion q = Quaternion::CreateFromYawPitchRoll(XMConvertToRadians(yaw), 0.0f, 0.0f);
	Vector3 p = Vector3::Transform(points[0], q);
	mMin = p;
	mMax = p;

	for (auto i = 1; i < points.size(); i++)
	{
		p = Vector3::Transform(points[i], q);
		updateMinMax(p);
	}
}

void AABB::UpdateWorldBox(const Vector3& position, float yaw)
{
	rotateY(yaw);
	mMin += position;
	mMax += position;
}
