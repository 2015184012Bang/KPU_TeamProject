#pragma once

class Box
{
public:
	static void Init();
	
	static Box& GetBox(string_view filename);

	void Update(const Vector3& position, float yaw);

public:
	const Vector3& GetMin() const { return mMin; }

	const Vector3& GetMax() const { return mMax; }

private:
	void load(string_view filename);
	void updateMinMax(const Vector3& point);
	void rotateY(float yaw);

private:
	Vector3 mMin = Vector3{ FLT_MAX, FLT_MAX, FLT_MAX };
	Vector3 mMax = Vector3{ FLT_MIN, FLT_MIN, FLT_MIN };

	static unordered_map<string, Box> sBoxes;
};

inline bool Intersects(const Box& a, const Box& b)
{
	const auto& aMin = a.GetMin();
	const auto& aMax = a.GetMax();
	const auto& bMin = b.GetMin();
	const auto& bMax = b.GetMax();

	bool no = aMax.x < bMin.x ||
		aMax.z < bMin.z ||
		bMax.x < aMin.x ||
		bMax.z < aMin.z;

	return !no;
}

