#pragma once
#include "Vector3.h"

class Vector4
{
public:
	float x, y, z, w;

	Vector4()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		w = 0.0f;
	}

	Vector4(float x, float y, float z, float w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	Vector4(float* v)
	{
		x = v[0];
		y = v[1];
		z = v[2];
		w = v[3];
	}

	Vector4(const float* v)
	{
		x = v[0];
		y = v[1];
		z = v[2];
		w = v[3];
	}

	Vector4(const Vector3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = 1.f;
	}

	inline float& operator[](int i)
	{
		return ((float*)this)[i];
	}

	inline float operator[](int i) const
	{
		return ((float*)this)[i];
	}

	Vector4 operator*(const Vector4& v)
	{
		float _w = w * v.w - x * v.x - y * v.y - z * v.z;
		float _x = x * v.w + w * v.x + y * v.z - z * v.y;
		float _y = y * v.w + w * v.y + z * v.x - x * v.z;
		float _z = z * v.w + w * v.z + x * v.y - y * v.x;

		return Vector4(_x, _y, _z, _w);
	}

	Vector4 operator*(const Vector3& v)
	{
		float _w = -x * v.x - y * v.y - z * v.z;
		float _x = w * v.x + y * v.z - z * v.y;
		float _y = w * v.y + z * v.x - x * v.z;
		float _z = w * v.z + x * v.y - y * v.x;

		return Vector4(_x, _y, _z, _w);
	}

	inline Vector4 Conjugate() const
	{
		return Vector4(-x, -y, -z, w);
	}

	inline Vector3 XYZ() const
	{
		return Vector3(x, y, z);
	}
};