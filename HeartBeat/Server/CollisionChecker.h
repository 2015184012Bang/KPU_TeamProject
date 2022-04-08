#pragma once

class Server;

/************************************************************************/
/* AABB                                                                 */
/************************************************************************/

class AABB
{
public:
	AABB();
	AABB(const wstring& path);

	void Load(const wstring& path);
	void UpdateWorldBox(const Vector3& position, float yaw);

	const Vector3& GetMin() const { return mMin; }
	const Vector3& GetMax() const { return mMax; }

private:
	void updateMinMax(const Vector3& point);
	void rotateY(float yaw);

	Vector3 mMin;
	Vector3 mMax;
};


/************************************************************************/
/* CollisionChecker                                                     */
/************************************************************************/

class CollisionChecker
{
public:
	CollisionChecker(Server* server);
	~CollisionChecker();

	void Update();

	const AABB* GetLocalBox(const wstring& name) const;

private:
	void processCollision(Entity& a, Entity& b);

private:
	unordered_map<wstring, AABB> mLocalBoxes;

	Server* mServer = nullptr;
};

