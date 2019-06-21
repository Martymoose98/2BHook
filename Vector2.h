#pragma once
#include <ostream>
#include "ImGui/imgui.h"

class Vector2
{
public:
	float x, y;

	Vector2();
	Vector2(float x, float y);
	Vector2(float xy);
	Vector2(float* xy);
	Vector2(const float* xy);
	//Vector2(POINT p)
	//	: x(p.x), y(p.y)
	//{
	//}

	inline Vector2& operator=(const Vector2& v);
	inline Vector2& operator=(const float* v);

	inline float& operator[](int i);
	inline float operator[](int i) const;

	inline Vector2 operator-() const;

	inline Vector2& operator+=(const Vector2& v);
	inline Vector2& operator-=(const Vector2& v);
	inline Vector2& operator*=(const Vector2& v);
	inline Vector2& operator/=(const Vector2& v);
	inline Vector2& operator+=(float f);
	inline Vector2& operator-=(float f);
	inline Vector2& operator*=(float f);
	inline Vector2& operator/=(float f);

	inline Vector2 operator+(const Vector2& v) const;
	inline Vector2 operator-(const Vector2& v) const;
	inline Vector2 operator*(const Vector2& v) const;
	inline Vector2 operator/(const Vector2& v) const;
	inline Vector2 operator+(float f) const;
	inline Vector2 operator-(float f) const;
	inline Vector2 operator*(float f) const;
	inline Vector2 operator/(float f) const;

	inline float Length() const;
	inline float LengthSqr() const;

	inline Vector2& Normalize();

	inline float Dot(const Vector2& v);

	inline bool IsZero() const;
	inline bool IsZeroTolerance(float tolerance = 0.01f) const;

	inline operator ImVec2&() { return *(ImVec2*)this; }

	inline friend std::ostream& operator<<(std::ostream& os, const Vector2& v);
};

inline Vector2& Vector2::operator=(const Vector2& v)
{
	x = v.x;
	y = v.y;
	return *this;
}

inline Vector2& Vector2::operator=(const float* v)
{
	x = v[0];
	y = v[1];
	return *this;
}

inline float& Vector2::operator[](int i)
{
	return ((float*)this)[i];
}

inline float Vector2::operator[](int i) const
{
	return ((float*)this)[i];
}

inline Vector2 Vector2::operator-() const
{
	return Vector2(-x, -y);
}

inline Vector2& Vector2::operator+=(const Vector2& v)
{
	x += v.x;
	y += v.y;
	return *this;
}

inline Vector2& Vector2::operator-=(const Vector2& v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

inline Vector2& Vector2::operator*=(const Vector2& v)
{
	x *= v.x;
	y *= v.y;
	return *this;
}

inline Vector2& Vector2::operator/=(const Vector2& v)
{
	x /= v.x;
	y /= v.y;
	return *this;
}

inline Vector2& Vector2::operator+=(float f)
{
	x += f;
	y += f;
	return *this;
}

inline Vector2& Vector2::operator-=(float f)
{
	x -= f;
	y -= f;
	return *this;
}

inline Vector2& Vector2::operator*=(float f)
{
	x *= f;
	y *= f;
	return *this;
}

inline Vector2& Vector2::operator/=(float f)
{
	x /= f;
	y /= f;
	return *this;
}

inline Vector2 Vector2::operator+(const Vector2& v) const
{
	return Vector2(x + v.x, y + v.y);
}

inline Vector2 Vector2::operator-(const Vector2& v) const
{
	return Vector2(x - v.x, y - v.y);
}

inline Vector2 Vector2::operator*(const Vector2& v) const
{
	return Vector2(x * v.x, y * v.y);
}

inline Vector2 Vector2::operator/(const Vector2& v) const
{
	return Vector2(x / v.x, y / v.y);
}

inline Vector2 Vector2::operator+(float f) const
{
	return Vector2(x + f, y + f);
}

inline Vector2 Vector2::operator-(float f) const
{
	return Vector2(x - f, y - f);
}

inline Vector2 Vector2::operator*(float f) const
{
	return Vector2(x * f, y * f);
}

inline Vector2 Vector2::operator/(float f) const
{
	return Vector2(x / f, y / f);
}
