#pragma once
#include <ostream>
#include <immintrin.h>
#include "Vector2.h"

float __forceinline __fastcall ssesqrt(float n);

class Matrix4x4;

class Vector3
{
public:
	float x, y, z;

	Vector3();
	Vector3(float X, float Y, float Z);
	Vector3(float XYZ);
	Vector3(float* v);
	Vector3(const float* v);
	Vector3(const __m128 v);

	inline operator float* (void);

	inline Vector3& operator=(const Vector3& v);
	inline Vector3& operator=(const float* v);

	inline float& operator[](int i);
	inline float operator[](int i) const;

	inline bool operator==(const Vector3& v) const;
	inline bool operator!=(const Vector3& v) const;

	inline Vector3& operator+=(const Vector3& v);
	inline Vector3& operator-=(const Vector3& v);
	inline Vector3& operator*=(const Vector3& v);
	inline Vector3& operator/=(const Vector3& v);
	inline Vector3& operator+=(float f);
	inline Vector3& operator-=(float f);
	inline Vector3& operator*=(float f);
	inline Vector3& operator/=(float f);

	inline Vector3 operator-() const;
	inline Vector3 operator+(const Vector3& v) const;
	inline Vector3 operator-(const Vector3& v) const;
	inline Vector3 operator*(const Vector3& v) const;
	inline Vector3 operator/(const Vector3& v) const;
	inline Vector3 operator+(float v) const;
	inline Vector3 operator-(float v) const;
	inline Vector3 operator*(float v) const;
	inline Vector3 operator/(float v) const;

	inline friend std::ostream& operator<<(std::ostream& os, const Vector3& v);

	inline float Length() const;
	inline float LengthSqr() const;
	inline float LengthXY() const;
	inline float LengthXZ() const;

	inline float DistTo(const Vector3& v) const;

	inline Vector3& Normalize();

	inline float Dot(const Vector3& v) const;
	inline float Dot(const float* v) const;

	inline Vector3 Cross(const Vector3& v) const;

	inline Vector3 Rotate(const Vector3& vAxis, float theta) const;
	inline Vector3 Rotate(const Matrix4x4& mRot) const;

	inline Vector2 XY() const;
	inline Vector2 XZ() const;
	inline Vector2 YZ() const;

	inline bool IsZero() const;
	inline bool IsZeroTolerance(float tolerance = 0.01f) const;
};
#include "Matrix4x4.h"
#include "Vector4.h"

inline Vector3& Vector3::operator=(const Vector3& v)
{
	x = v.x; y = v.y; z = v.z; return *this;
}

inline Vector3& Vector3::operator=(const float* v)
{
	x = v[0]; y = v[1]; z = v[2]; return *this;
}

inline float& Vector3::operator[](int i)
{
	return ((float*)this)[i];
}

inline float Vector3::operator[](int i) const
{
	return ((float*)this)[i];
}

inline bool Vector3::operator==(const Vector3& v) const
{
	return (x == v.x && x == v.y && z == v.y);
}

inline bool Vector3::operator!=(const Vector3& v) const
{
	return (x != v.x || y != v.y || z != v.z);
}

inline Vector3& Vector3::operator+=(const Vector3& v)
{
	x += v.x; y += v.y; z += v.z; return *this;
}

inline Vector3& Vector3::operator-=(const Vector3& v)
{
	x -= v.x; y -= v.y; z -= v.z; return *this;
}

inline Vector3& Vector3::operator*=(const Vector3& v)
{
	x *= v.x; y *= v.y; z *= v.z; return *this;
}

inline Vector3& Vector3::operator/=(const Vector3& v)
{
	x /= v.x; y /= v.y; z /= v.z; return *this;
}

inline Vector3& Vector3::operator+=(float f)
{
	x += f; y += f; z += f; return *this;
}

inline Vector3& Vector3::operator-=(float f)
{
	x -= f; y -= f; z -= f; return *this;
}

inline Vector3& Vector3::operator*=(float f)
{
	x *= f; y *= f; z *= f; return *this;
}

inline Vector3& Vector3::operator/=(float v)
{
	x /= v; y /= v; z /= v; return *this;
}

inline Vector3 Vector3::operator-() const
{
	return Vector3(-x, -y, -z);
}

inline Vector3 Vector3::operator+(const Vector3& v) const
{
	return Vector3(x + v.x, y + v.y, z + v.z);
}

inline Vector3 Vector3::operator-(const Vector3& v) const
{
	return Vector3(x - v.x, y - v.y, z - v.z);
}

inline Vector3 Vector3::operator*(const Vector3& v) const
{
	return Vector3(x * v.x, y * v.y, z * v.z);
}

inline Vector3 Vector3::operator/(const Vector3& v) const
{
	return Vector3(x / v.x, y / v.y, z / v.z);
}

inline Vector3 Vector3::operator+(float v) const
{
	return Vector3(x + v, y + v, z + v);
}

inline Vector3 Vector3::operator-(float v) const
{
	return Vector3(x - v, y - v, z - v);
}

inline Vector3 Vector3::operator*(float v) const
{
	return Vector3(x * v, y * v, z * v);
}

inline Vector3 Vector3::operator/(float v) const
{
	return Vector3(x / v, y / v, z / v);
}

inline float Vector3::Length() const
{
	return ssesqrt(x * x + y * y + z * z);
}

inline float Vector3::LengthSqr() const
{
	return (x * x + y * y + z * z);
}

inline float Vector3::LengthXY() const
{
	return ssesqrt(x * x + y * y);
}

inline float Vector3::LengthXZ() const
{
	return ssesqrt(x * x + z * z);
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

inline float Vector3::DistTo(const Vector3& v) const
{
	return (*this - v).Length();
}

inline float Vector3::Dot(const Vector3& v) const
{
	return (x * v.x + y * v.y + z * v.z);
}

inline float Vector3::Dot(const float* v) const
{
	return (x * v[0] + y * v[1] + z * v[2]);
}

inline Vector3 Vector3::Cross(const Vector3& v) const
{
	return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}

inline Vector3 Vector3::Rotate(const Vector3& vAxis, float angle) const
{
	float SinHalfAngle = sinf(angle / 2.0f);
	float CosHalfAngle = cosf(angle / 2.0f);

	Vector4 Rotation(vAxis.x * SinHalfAngle, vAxis.y * SinHalfAngle, vAxis.z * SinHalfAngle, CosHalfAngle);
	Vector4 Conjugate = Rotation.Conjugate();
	Vector4 Rotated = Rotation * (*this) * Conjugate;

	return Rotated.XYZ();
}

inline Vector3 Vector3::Rotate(const Matrix4x4& mRot) const
{
	return Vector3(Dot(mRot[0]), Dot(mRot[1]), Dot(mRot[2]));
}

inline Vector3& Vector3::Normalize()
{
	float l = this->Length();

	if (l != 0.0f)
	{
		*this /= l;
	}
	else
	{
		x = 0.0f;
		y = 0.0f;
		z = 1.0f;
	}
	return *this;
}

class __declspec(align(16)) Vector3Aligned : public Vector3
{
public:
	Vector3Aligned()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	//	w = 1.0f;
	}

	Vector3Aligned(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		//this->w = 1.0f;
	}

	Vector3Aligned(const Vector3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		//w = 1.0f;
	}

	Vector3Aligned(const Vector3Aligned& v)
	{
		*this = v;
	}

	Vector3Aligned(const __m128 v)
	{
		*(__m128*)this = v;
	}

	inline operator __m128()
	{
		return *(__m128*)this;
	}

//private:
	//float w;
};
