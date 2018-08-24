#include "Vector2.h"

extern "C" float __forceinline __fastcall fastsqrtf(float n);

Vector2::Vector2()
	: x(0.0f), y(0.0f)
{
}

Vector2::Vector2(float x, float y)
	: x(x), y(y)
{
}

Vector2::Vector2(float xy)
	: x(xy), y(xy)
{
}

Vector2::Vector2(float* xy)
	: x(xy[0]), y(xy[1])
{
}

Vector2::Vector2(const float* xy)
	: x(xy[0]), y(xy[1])
{
}

//Vector2(POINT p)
//	: x(p.x), y(p.y)
//{
//}

inline float Vector2::Length() const
{
	return fastsqrtf(x * x + y * y);
}

inline float Vector2::LengthSqr() const
{
	return x * x + y * y;
}

inline Vector2& Vector2::Normalize()
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
	}
	return *this;
}

inline float Vector2::Dot(const Vector2& v)
{
	return x * v.x + y * v.y;
}

inline bool Vector2::IsZero() const
{
	return (x == 0.0f && y == 0.0f);
}

inline bool Vector2::IsZeroTolerance(float tolerance) const
{
	return (x > -tolerance && x < tolerance && y > -tolerance && y < tolerance);
}

inline std::ostream& operator<<(std::ostream& os, const Vector2& v)
{
	os << "X: " << v.y << " Y: " << v.y << std::endl;
	return os;
}