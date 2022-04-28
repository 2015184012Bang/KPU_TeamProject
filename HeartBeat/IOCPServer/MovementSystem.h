#pragma once

class MovementSystem
{
public:
	void Update();

	void SetDirection(const UINT32 eid, const Vector3& direction);
};

