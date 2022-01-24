#pragma once

#include "Resource.h"

class AABB : public IResource
{
public:
	AABB();

	virtual void Load(const wstring& path) override;
	void UpdateWorldBox(const Vector3& position, float yaw);

	const Vector3& GetMin() const { return mMin; }
	const Vector3& GetMax() const { return mMax; }

private:
	void updateMinMax(const Vector3& point);
	void rotateY(float yaw);

private:
	Vector3 mMin;
	Vector3 mMax;
};

