#pragma once

class AABB
{
public:
	AABB();
	AABB(const string& path);

	void Load(const string& path);
	void UpdateWorldBox(const Vector3& position, float yaw);

	const Vector3& GetMin() const { return mMin; }
	const Vector3& GetMax() const { return mMax; }

	void SetMin(const Vector3& min) { mMin = min; }
	void SetMax(const Vector3& max) { mMax = max; }

private:
	void updateMinMax(const Vector3& point);
	void rotateY(float yaw);

private:
	Vector3 mMin;
	Vector3 mMax;
};
