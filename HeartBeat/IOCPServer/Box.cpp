#include "pch.h"
#include "Box.h"

#include <sstream>
#include <fstream>

#include <rapidjson/document.h>

#include "Values.h"


std::unordered_map<std::string, Box> Box::sBoxes;

void Box::Update(const Vector3& position, float yaw)
{
	rotateY(yaw);
	mMin += position;
	mMax += position;
}

void Box::Init()
{
	GetBox("../Assets/Boxes/Cart.box");
	GetBox("../Assets/Boxes/Cell.box");
	GetBox("../Assets/Boxes/Cube.box");
	GetBox("../Assets/Boxes/Character.box");
	GetBox("../Assets/Boxes/Dog.box");
	GetBox("../Assets/Boxes/Tank.box");
	GetBox("../Assets/Boxes/Virus.box");
	GetBox("../Assets/Boxes/House.box");

	// 플레이어가 공격할 때 사용할 히트박스 생성
	Box hitbox;
	hitbox.SetMin(Vector3{ -100.0f, 0.0f, 0.0f });
	hitbox.SetMax(Vector3{ 100.0f, 0.0f, Values::BaseAttackRange });
	Box::SetBox(hitbox, "Hitbox");
}

Box& Box::GetBox(string_view filename)
{
	auto boxFile = fs::path(filename).filename();

	if (auto iter = sBoxes.find(boxFile.string()); iter != sBoxes.end())
	{
		return iter->second;
	}
	else
	{
		Box box = {};
		box.load(filename);
		sBoxes[boxFile.string()] = box;
		return sBoxes[boxFile.string()];
	}
}

void Box::SetBox(const Box& box, string_view name)
{
	if (auto iter = sBoxes.find(name.data()); iter != sBoxes.end())
	{
		LOG("Box already exists");
		return;
	}

	sBoxes[name.data()] = box;
}

void Box::load(string_view filename)
{
	ifstream file(filename.data());

	ASSERT(file.is_open(), "Could not open file: {0}", filename.data());

	std::stringstream fileStream;
	fileStream << file.rdbuf();
	string contents = fileStream.str();
	rapidjson::StringStream jsonStr(contents.c_str());
	rapidjson::Document doc;
	doc.ParseStream(jsonStr);

	ASSERT(doc.IsObject(), "Its not valid json file.");

	const rapidjson::Value& vertsJson = doc["vertices"];
	for (rapidjson::SizeType i = 0; i < vertsJson.Size(); ++i)
	{
		const rapidjson::Value& vert = vertsJson[i];

		ASSERT(vert.IsArray(), "Invalid vertex format");

		Vector3 point;

		point.x = vert[0].GetFloat();
		point.y = vert[1].GetFloat();
		point.z = vert[2].GetFloat();

		updateMinMax(point);
	}
}


void Box::updateMinMax(const Vector3& point)
{
	mMin.x = min(mMin.x, point.x);
	mMin.y = min(mMin.y, point.y);
	mMin.z = min(mMin.z, point.z);
	mMax.x = max(mMax.x, point.x);
	mMax.y = max(mMax.y, point.y);
	mMax.z = max(mMax.z, point.z);
}

void Box::rotateY(float yaw)
{
	array<Vector3, 8> points;

	points[0] = mMin;
	points[1] = Vector3(mMax.x, mMin.y, mMin.z);
	points[2] = Vector3(mMin.x, mMax.y, mMin.z);
	points[3] = Vector3(mMin.x, mMin.y, mMax.z);
	points[4] = Vector3(mMin.x, mMax.y, mMax.z);
	points[5] = Vector3(mMax.x, mMin.y, mMax.z);
	points[6] = Vector3(mMax.x, mMax.y, mMin.z);
	points[7] = mMax;

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
