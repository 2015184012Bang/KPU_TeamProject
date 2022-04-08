#include "ServerPCH.h"
#include "CollisionChecker.h"

#include <rapidjson/document.h>

#include "HeartBeat/Tags.h"

#include "Server.h"
#include "ServerComponents.h"
#include "ServerSystems.h"

/************************************************************************/
/* AABB                                                                 */
/************************************************************************/

AABB::AABB(const wstring& path)
	: mMin(FLT_MAX, FLT_MAX, FLT_MAX)
	, mMax(FLT_MIN, FLT_MIN, FLT_MIN)
{
	Load(path);
}

AABB::AABB()
	: mMin(FLT_MAX, FLT_MAX, FLT_MAX)
	, mMax(FLT_MIN, FLT_MIN, FLT_MIN)
{

}

void AABB::Load(const wstring& path)
{
	std::ifstream file(path);

	if (!file.is_open())
	{
		HB_ASSERT(false, "Could not open file: {0}", ws2s(path));
	}

	std::stringstream fileStream;
	fileStream << file.rdbuf();
	string contents = fileStream.str();
	rapidjson::StringStream jsonStr(contents.c_str());
	rapidjson::Document doc;
	doc.ParseStream(jsonStr);

	if (!doc.IsObject())
	{
		HB_ASSERT(false, "{0} is not valid json file!", ws2s(path));
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

/************************************************************************/
/* CollisionChecker                                                     */
/************************************************************************/

CollisionChecker::CollisionChecker(Server* server)
	: mServer(server)
{
	mLocalBoxes.try_emplace(L"Cell", L"../Assets/Boxes/Cell.box");
	mLocalBoxes.try_emplace(L"Cube", L"../Assets/Boxes/Cube.box");
	mLocalBoxes.try_emplace(L"Character", L"../Assets/Boxes/Character.box");
	mLocalBoxes.try_emplace(L"Virus", L"../Assets/Boxes/Virus.box");
}

CollisionChecker::~CollisionChecker()
{
	mLocalBoxes.clear();
}

void CollisionChecker::Update()
{
	auto view = mServer->GetRegistry().view<SBoxComponent>();

	for (auto id : view)
	{
		Entity entity = Entity(id, mServer);

		auto& transform = entity.GetComponent<STransformComponent>();
		auto& box = entity.GetComponent<SBoxComponent>();

		ServerSystems::UpdateBox(box.LocalBox, &box.MyBox, transform.Position, transform.Rotation.y);
	}

	auto players = mServer->GetRegistry().view<Tag_Player>();
	auto enemies = mServer->GetRegistry().view<Tag_Enemy>();

	for (auto player : players)
	{
		Entity p = Entity(player, mServer);
		auto& playerBox = p.GetComponent<SBoxComponent>();

		for (auto enemy : enemies)
		{
			Entity e = Entity(enemy, mServer);
			auto& enemyBox = e.GetComponent<SBoxComponent>();

			if (ServerSystems::Intersects(playerBox.MyBox, enemyBox.MyBox))
			{
				HB_LOG("Collision!!!");
				//processCollision(p, e);
			}
		}
	}
}

const AABB* CollisionChecker::GetLocalBox(const wstring& name) const
{
	auto iter = mLocalBoxes.find(name);

	if (iter != mLocalBoxes.end())
	{
		return &iter->second;
	}
	else
	{
		HB_LOG("No box : {0}", ws2s(name));
		return  nullptr;
	}
}

void CollisionChecker::processCollision(Entity& a, Entity& b)
{
	auto& aBox = a.GetComponent<SBoxComponent>();
	auto& bBox = b.GetComponent<SBoxComponent>();

	const Vector3& aMax = aBox.MyBox.GetMax();
	const Vector3& aMin = aBox.MyBox.GetMin();

	const Vector3& bMax = bBox.MyBox.GetMax();
	const Vector3& bMin = bBox.MyBox.GetMin();

	float dx1 = aMax.x - bMin.x;
	float dx2 = aMin.x - bMax.x;
	float dy1 = aMax.y - bMin.y;
	float dy2 = aMin.y - bMax.y;
	float dz1 = aMax.z - bMin.z;
	float dz2 = aMin.z - bMax.z;

	float dx = abs(dx1) < abs(dx2) ?
		dx1 : dx2;
	float dy = abs(dy1) < abs(dy2) ?
		dy1 : dy2;
	float dz = abs(dz1) < abs(dz2) ?
		dz1 : dz2;

	STransformComponent& transform = a.GetComponent<STransformComponent>();
	
	if (abs(dx) <= abs(dz))
	{
		transform.Position.x += dx;
	}
	else if (abs(dy) <= abs(dx) && abs(dy) <= abs(dz))
	{
		transform.Position.y += dy;
	}
	else
	{
		transform.Position.z += dz;
	}

	ServerSystems::UpdateBox(aBox.LocalBox, &aBox.MyBox, transform.Position, transform.Rotation.y);
}
