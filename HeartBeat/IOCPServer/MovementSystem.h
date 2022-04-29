#pragma once

class GameManager;

class MovementSystem
{
public:
	MovementSystem(shared_ptr<GameManager>&& gm);

	void Update();

	void SetDirection(const UINT32 eid, const Vector3& direction);

private:
	shared_ptr<GameManager> mGameManager = nullptr;
};

