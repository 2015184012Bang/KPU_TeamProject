#pragma once

class Server;

/************************************************************************/
/* CollisionChecker                                                     */
/************************************************************************/

class CollisionChecker
{
public:
	CollisionChecker(Server* server);
	~CollisionChecker();

	void Update();
	void MakeHitBoxAndCheck(const Vector3& position, float yaw);

	const AABB* GetLocalBox(const wstring& name) const;

private:
	void processCollision(Entity& a, Entity& b);

private:
	unordered_map<wstring, AABB> mLocalBoxes;

	Server* mServer = nullptr;
};
