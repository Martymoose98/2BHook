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


inline std::ostream& operator<<(std::ostream& os, const Vector3& v)
{
	os << "X: " << v.x << " Y: " << v.y << " Z: " << v.z << std::endl;
	return os;
}

/*inline Vector3 Rotate(const Vector3& vAxis, float angle) const
{
	float SinHalfAngle = sinf(DEGTORAD(angle / 2));
	float CosHalfAngle = cosf(DEGTORAD(angle / 2));

	Vector4 Rotation(vAxis.x * SinHalfAngle, vAxis.y * SinHalfAngle, vAxis.z * SinHalfAngle, CosHalfAngle);
	Vector4 Conjugate = Rotation.Conjugate();
	Vector4 Rotated = Rotation * (*this) * Conjugate;

	return Rotated.XYZ();
}*/

Vector3 Vector3::Rotate(const Matrix4x4& mRot) const
{
	return Vector3(Dot(mRot[0]), Dot(mRot[1]), Dot(mRot[2]));
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

inline bool Vector3::IsZero() const
{
	return (x == 0.0f && y == 0.0f && z == 0.0f);
}

inline bool Vector3::IsZeroTolerance(float tolerance) const
{
	return (x > -tolerance && x < tolerance
		&&	y > -tolerance && y < tolerance
		&&	z > -tolerance && z < tolerance);
}