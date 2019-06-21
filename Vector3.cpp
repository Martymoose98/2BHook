#include "Vector3.h"

Vector3::Vector3()
{
	x = y = z = 0.0f;
}

Vector3::Vector3(float X, float Y, float Z)
{
	x = X; y = Y; z = Z;
}

Vector3::Vector3(float XYZ)
{
	x = XYZ; y = XYZ; z = XYZ;
}

Vector3::Vector3(float* v)
{
	x = v[0]; y = v[1]; z = v[2];
}

Vector3::Vector3(const float* v)
{
	x = v[0]; y = v[1]; z = v[2];
}

Vector3::Vector3(const __m128 v)
{
	x = v.m128_f32[0];
	y = v.m128_f32[1];
	z = v.m128_f32[2];
}

inline std::ostream& operator<<(std::ostream& os, const Vector3& v)
{
	os << "X: " << v.x << " Y: " << v.y << " Z: " << v.z << std::endl;
	return os;
}

inline Vector2 Vector3::XY() const
{
	return Vector2(x, y);
}

inline Vector2 Vector3::XZ() const
{
	return Vector2(x, z);
}

inline Vector2 Vector3::YZ() const
{
	return Vector2(y, z);
}