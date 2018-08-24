#pragma once
#include <ostream>
#include <immintrin.h>
#include "Vector3.h"

class Vector3;

/*
	*
	* 	rotation z axis matrix:
	* 	----    		     ----
	* 	|  cos(z), sin(z), 0, 0 |
	* 	| -sin(z), cos(z), 0, 0 |
	* 	|     0, 	    0, 1, 0 |
	* 	|     0, 		0, 0, 1 |
	* 	----                 ----
	*
	*  rotation y axis matrix:
	* 	----    		     ----
	* 	| cos(y), 0, -sin(y), 0 |
	* 	| 	   0, 1,       0, 0 |
	* 	| sin(y), 0,  cos(y), 0 |
	* 	|      0, 0,       0, 1 |
	* 	----                 ----
	*
	* 	rotation x axis matrix:
	*  ----	  	   		     ----
	*  | 1, 	 0, 	  0,  0 |
	*  | 0,  cos(x), sin(x),  0 |
	*  | 0, -sin(x), cos(x),  0	|
	*  | 0,      0,       0,  1 |
	*  ----				     ----
	*  reference = http://mathworld.wolfram.com/RotationMatrix.html
	*/
class Matrix4x4
{
public:
	union
	{
		struct
		{
			float _00, _01, _02, _03;
			float _10, _11, _12, _13;
			float _20, _21, _22, _23;
			float _30, _31, _32, _33;
		};

		float m[4][4];
		float mm[16];

		struct
		{
			__m128 m0, m1, m2, m3;
		};
	};

	Matrix4x4();
	Matrix4x4(const Matrix4x4& mOther);
	Matrix4x4(const __m128& m0, const __m128& m1, const __m128& m2, const __m128& m3);
	Matrix4x4(float _00, float _01, float _02, float _03,
		float _10, float _11, float _12, float _13,
		float _20, float _21, float _22, float _23,
		float _30, float _31, float _32, float _33);

	inline Matrix4x4& operator*(const Matrix4x4& matrix);

	inline float* operator[](int i);
	inline const float* operator[](int i) const;

	inline float& operator()(int row, int col);
	inline float operator()(int row, int col) const;

	inline void Transpose();
	inline Vector3& Transform(const Vector3& v);
	inline Vector3& GetAxis(int i);
	inline friend std::ostream& operator<<(std::ostream& os, const Matrix4x4& m);
};
typedef Matrix4x4 VMatrix;

inline Matrix4x4& Matrix4x4::operator*(const Matrix4x4& matrix)
{
	m[0][0] = m[0][0] * matrix.m[0][0] + m[1][0] * matrix.m[0][1] + m[2][0] * matrix.m[0][2] + m[3][0] * matrix.m[0][3];
	m[0][1] = m[0][1] * matrix.m[0][0] + m[1][1] * matrix.m[0][1] + m[2][1] * matrix.m[0][2] + m[3][1] * matrix.m[0][3];
	m[0][2] = m[0][2] * matrix.m[0][0] + m[1][2] * matrix.m[0][1] + m[2][2] * matrix.m[0][2] + m[3][2] * matrix.m[0][3];
	m[0][3] = m[0][3] * matrix.m[0][0] + m[1][3] * matrix.m[0][1] + m[2][3] * matrix.m[0][2] + m[3][3] * matrix.m[0][3];

	m[1][0] = m[0][0] * matrix.m[1][0] + m[1][0] * matrix.m[1][1] + m[2][0] * matrix.m[1][2] + m[3][0] * matrix.m[1][3];
	m[1][1] = m[0][1] * matrix.m[1][0] + m[1][1] * matrix.m[1][1] + m[2][1] * matrix.m[1][2] + m[3][1] * matrix.m[1][3];
	m[1][2] = m[0][2] * matrix.m[1][0] + m[1][2] * matrix.m[1][1] + m[2][2] * matrix.m[1][2] + m[3][2] * matrix.m[1][3];
	m[1][3] = m[0][3] * matrix.m[1][0] + m[1][3] * matrix.m[1][1] + m[2][3] * matrix.m[1][2] + m[3][3] * matrix.m[1][3];

	m[2][0] = m[0][0] * matrix.m[2][0] + m[1][0] * matrix.m[2][1] + m[2][0] * matrix.m[2][2] + m[3][0] * matrix.m[2][3];
	m[2][1] = m[0][1] * matrix.m[2][0] + m[1][1] * matrix.m[2][1] + m[2][1] * matrix.m[2][2] + m[3][1] * matrix.m[2][3];
	m[2][2] = m[0][2] * matrix.m[2][0] + m[1][2] * matrix.m[2][1] + m[2][2] * matrix.m[2][2] + m[3][2] * matrix.m[2][3];
	m[2][3] = m[0][3] * matrix.m[2][0] + m[1][3] * matrix.m[2][1] + m[2][3] * matrix.m[2][2] + m[3][3] * matrix.m[2][3];

	m[3][0] = m[0][0] * matrix.m[3][0] + m[1][0] * matrix.m[3][1] + m[2][0] * matrix.m[3][2] + m[3][0] * matrix.m[3][3];
	m[3][1] = m[0][1] * matrix.m[3][0] + m[1][1] * matrix.m[3][1] + m[2][1] * matrix.m[3][2] + m[3][1] * matrix.m[3][3];
	m[3][2] = m[0][2] * matrix.m[3][0] + m[1][2] * matrix.m[3][1] + m[2][2] * matrix.m[3][2] + m[3][2] * matrix.m[3][3];
	m[3][3] = m[0][3] * matrix.m[3][0] + m[1][3] * matrix.m[3][1] + m[2][3] * matrix.m[3][2] + m[3][3] * matrix.m[3][3];

	return *this;
}

inline float* Matrix4x4::operator[](int i)
{
	return m[i];
}

inline const float* Matrix4x4::operator[](int i) const
{
	return m[i];
}

inline float& Matrix4x4::operator()(int row, int col)
{
	return m[row][col];
}

inline float Matrix4x4::operator()(int row, int col) const
{
	return m[row][col];
}

inline void Matrix4x4::Transpose()
{
	__m128 _Tmp3, _Tmp2, _Tmp1, _Tmp0;

	_Tmp0 = _mm_shuffle_ps(m0, m1, 0x44);
	_Tmp2 = _mm_shuffle_ps(m0, m1, 0xEE);
	_Tmp1 = _mm_shuffle_ps(m2, m3, 0x44);
	_Tmp3 = _mm_shuffle_ps(m2, m3, 0xEE);
	m0 = _mm_shuffle_ps(_Tmp0, _Tmp1, 0x88);
	m1 = _mm_shuffle_ps(_Tmp0, _Tmp1, 0xDD);
	m2 = _mm_shuffle_ps(_Tmp2, _Tmp3, 0x88);
	m3 = _mm_shuffle_ps(_Tmp2, _Tmp3, 0xDD);
}

inline Vector3& Matrix4x4::GetAxis(int i)
{
	return *(Vector3*)((*this)[i]);
}