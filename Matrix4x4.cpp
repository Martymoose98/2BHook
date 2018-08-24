#include "Matrix4x4.h"

Matrix4x4::Matrix4x4()
	: m0(_mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f)), m1(_mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f)),
	m2(_mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f)), m3(_mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f))
{
}

Matrix4x4::Matrix4x4(const Matrix4x4& mOther)
{
	*this = mOther;
}

Matrix4x4::Matrix4x4(const __m128& m0, const __m128& m1, const __m128& m2, const __m128& m3)
	: m0(m0), m1(m1), m2(m2), m3(m3)
{
}

Matrix4x4::Matrix4x4(float _00, float _01, float _02, float _03,
	float _10, float _11, float _12, float _13,
	float _20, float _21, float _22, float _23,
	float _30, float _31, float _32, float _33)
	: _00(_00), _01(_01), _02(_02), _03(_03),
	_10(_10), _11(_11), _12(_12), _13(_13),
	_20(_20), _21(_21), _22(_22), _23(_23),
	_30(_30), _31(_31), _32(_32), _33(_33)
{
}

inline Vector3& Matrix4x4::Transform(const Vector3& v)
{
	return Vector3(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3], m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3], m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3]);
}

inline std::ostream& operator<<(std::ostream& os, const Matrix4x4& m)
{
	os << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << ", " << m[0][3] << std::endl
		<< m[1][0] << ", " << m[1][1] << ", " << m[1][2] << ", " << m[1][3] << std::endl
		<< m[2][0] << ", " << m[2][1] << ", " << m[2][2] << ", " << m[2][3] << std::endl
		<< m[3][0] << ", " << m[3][1] << ", " << m[3][2] << ", " << m[3][3] << std::endl;

	return os;
}