#include "Matrix4x4.h"

Matrix4x4::Matrix4x4()
	: m0(_mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f)), m1(_mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f)),
	m2(_mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f)), m3(_mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f))
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

void Matrix4x4::InitAxisAngle(const Vector3& vAxis, float theta)
{
	float sin, cos, c;
	Math::SinCos(theta, &sin, &cos);
	c = 1.f - cos;

	m[0][0] = (vAxis.x * vAxis.x) * c + cos;
	m[0][1] = (vAxis.x * vAxis.y) * c - vAxis.z * sin;
	m[0][2] = (vAxis.x * vAxis.z) * c + vAxis.y * sin;

	m[1][0] = (vAxis.y * vAxis.x) * c + vAxis.z * sin;
	m[1][1] = (vAxis.y * vAxis.y) * c + cos;
	m[1][2] = (vAxis.y * vAxis.z) * c - vAxis.x * sin;

	m[2][0] = (vAxis.z * vAxis.x) * c - vAxis.y * sin;
	m[2][1] = (vAxis.z * vAxis.y) * c + vAxis.x * sin;
	m[2][2] = (vAxis.z * vAxis.z) * c + cos;
}

void Matrix4x4::InitTransform(const Vector3& vAngles, const Vector3& vPosition)
{
	Vector3 vForward, vRight, vUp;
	Math::AngleVectors(vAngles, &vForward, &vRight, &vUp);
	InitTransform(vForward, vRight, vUp, vPosition);
}

void Matrix4x4::InitTransform(const Vector3& vForward, const Vector3& vRight, const Vector3& vUp, const Vector3& vPosition)
{
	m[RIGHT][0] = vRight.x;
	m[RIGHT][1] = vRight.y;
	m[RIGHT][2] = vRight.z;
	m[RIGHT][3] = 0.0f;

	m[UP][0] = vUp.x;
	m[UP][1] = vUp.y;
	m[UP][2] = vUp.z;
	m[UP][3] = 0.0f;

	m[FORWARD][0] = vForward.x;
	m[FORWARD][1] = vForward.y;
	m[FORWARD][2] = vForward.z;
	m[FORWARD][3] = 0.0f;

	m[POSITION][0] = vPosition.x;
	m[POSITION][1] = vPosition.y;
	m[POSITION][2] = vPosition.z;
	m[POSITION][3] = 1.0f;
}

inline float Matrix4x4::ThetaOfAxisAngle() const
{
	Vector3 v(m[2][1] - m[1][2], m[0][2] - m[2][0], m[1][2] - m[0][1]);
	return atan2f(v.Length(), (m[0][0] + m[1][1] + m[2][2]) - 1.f);
}

inline std::ostream& operator<<(std::ostream& os, const Matrix4x4& m)
{
	os << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << ", " << m[0][3] << std::endl
		<< m[1][0] << ", " << m[1][1] << ", " << m[1][2] << ", " << m[1][3] << std::endl
		<< m[2][0] << ", " << m[2][1] << ", " << m[2][2] << ", " << m[2][3] << std::endl
		<< m[3][0] << ", " << m[3][1] << ", " << m[3][2] << ", " << m[3][3] << std::endl;

	return os;
}