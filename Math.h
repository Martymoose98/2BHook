#pragma once

#include <Windows.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <ostream>
#include <time.h>
#include <math.h>

#define M_RADPI 57.295779513082f
#define M_PI 3.14159265358979323846
#define M_PI_F		((float)(M_PI))

#define SQUARE(a) (a) * (a)
#define CUBE(a) (a) * (a) * (a)
#define DEGTORAD(x)  ((float)(x) * (float)( M_PI_F / 180.f ))
#define RADTODEG(x)  ((float)(x) * (float)( 180.f / M_PI_F ))

/*
http://www.codeproject.com/Articles/69941/Best-Square-Root-Method-Algorithm-Function-Precisi

Phone can suck a dick, this is the fastest square root possible on a computer without needing to quantum compute
*/

EXTERN_C double __forceinline __fastcall fastsqrt(double n);
EXTERN_C float __forceinline __fastcall fastsqrtf(float n);
EXTERN_C void __forceinline __fastcall sincos(float radians, float* sine, float* cosine);

double __forceinline __fastcall ssesqrt(double n)
{
	return _mm_cvtsd_f64(_mm_sqrt_pd(_mm_set_sd(n))); // FIXME? _mm_sqrt_pd - computes the square root of packed double-precision (64-bit) floating-point elements in a, and store the results in dst. i only need the first element
}

//Supa fast SSE float square root
float __forceinline __fastcall ssesqrt(float n)
{
	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(n)));
}

class Vector2
{
public:
	float x, y;

	Vector2()
		: x(0.0f), y(0.0f)
	{
	}

	Vector2(float x, float y)
		: x(x), y(y)
	{
	}

	Vector2(float xy)
		: x(xy), y(xy)
	{
	}

	Vector2(float* xy)
		: x(xy[0]), y(xy[1])
	{
	}

	Vector2(const float* xy)
		: x(xy[0]), y(xy[1])
	{
	}

	Vector2(POINT p)
		: x(p.x), y(p.y)
	{
	}

	inline Vector2& operator=(const Vector2& v)
	{
		x = v.x;
		y = v.y;
		return *this;
	}

	inline Vector2& operator=(const float* v)
	{
		x = v[0];
		y = v[1];
		return *this;
	}

	inline float& operator[](int i)
	{
		return ((float*)this)[i];
	}

	inline float operator[](int i) const
	{
		return ((float*)this)[i];
	}

	inline Vector2 operator-() const
	{
		return Vector2(-x, -y);
	}

	inline Vector2& operator+=(const Vector2& v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	inline Vector2& operator-=(const Vector2& v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	inline Vector2& operator*=(const Vector2& v)
	{
		x *= v.x;
		y *= v.y;
		return *this;
	}

	inline Vector2& operator/=(const Vector2& v)
	{
		x /= v.x;
		y /= v.y;
		return *this;
	}

	inline Vector2& operator+=(float f)
	{
		x += f;
		y += f;
		return *this;
	}

	inline Vector2& operator-=(float f)
	{
		x -= f;
		y -= f;
		return *this;
	}

	inline Vector2& operator*=(float f)
	{
		x *= f;
		y *= f;
		return *this;
	}

	inline Vector2& operator/=(float f)
	{
		x /= f;
		y /= f;
		return *this;
	}

	inline Vector2 operator+(const Vector2& v) const
	{
		return Vector2(x + v.x, y + v.y);
	}

	inline Vector2 operator-(const Vector2& v) const
	{
		return Vector2(x - v.x, y - v.y);
	}

	inline Vector2 operator*(const Vector2& v) const
	{
		return Vector2(x * v.x, y * v.y);
	}

	inline Vector2 operator/(const Vector2& v) const
	{
		return Vector2(x / v.x, y / v.y);
	}

	inline Vector2 operator+(float f) const
	{
		return Vector2(x + f, y + f);
	}

	inline Vector2 operator-(float f) const
	{
		return Vector2(x - f, y - f);
	}

	inline Vector2 operator*(float f) const
	{
		return Vector2(x * f, y * f);
	}

	inline Vector2 operator/(float f) const
	{
		return Vector2(x / f, y / f);
	}

	inline float Length() const
	{
		return fastsqrtf(x * x + y * y);
	}

	inline float LengthSqr() const
	{
		return x * x + y * y;
	}

	inline Vector2& Normalize()
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

	inline float Dot(const Vector2& v)
	{
		return x * v.x + y * v.y;
	}

	inline bool IsZero() const
	{
		return (x == 0.0f && y == 0.0f);
	}

	inline bool IsZeroTolerance(float tolerance = 0.01f) const
	{
		return (x > -tolerance && x < tolerance && y > -tolerance && y < tolerance);
	}

	inline friend std::ostream& operator<<(std::ostream& os, const Vector2& v)
	{
		os << "X: " << v.y << " Y: " << v.y << std::endl;
		return os;
	}
};

class Vector3
{
public:
	float x, y, z;

	Vector3()
	{
		x = y = z = 0.0f;
	}

	Vector3(float X, float Y, float Z)
	{
		x = X; y = Y; z = Z;
	}

	Vector3(float XYZ)
	{
		x = XYZ; y = XYZ; z = XYZ;
	}

	Vector3(float* v)
	{
		x = v[0]; y = v[1]; z = v[2];
	}

	Vector3(const float* v)
	{
		x = v[0]; y = v[1]; z = v[2];
	}

	inline Vector3& operator=(const Vector3& v)
	{
		x = v.x; y = v.y; z = v.z; return *this;
	}

	inline Vector3& operator=(const float* v)
	{
		x = v[0]; y = v[1]; z = v[2]; return *this;
	}

	inline float& operator[](int i)
	{
		return ((float*)this)[i];
	}

	inline float operator[](int i) const
	{
		return ((float*)this)[i];
	}

	inline Vector3& operator+=(const Vector3& v)
	{
		x += v.x; y += v.y; z += v.z; return *this;
	}

	inline Vector3& operator-=(const Vector3& v)
	{
		x -= v.x; y -= v.y; z -= v.z; return *this;
	}

	inline Vector3& operator*=(const Vector3& v)
	{
		x *= v.x; y *= v.y; z *= v.z; return *this;
	}

	inline Vector3& operator/=(const Vector3& v)
	{
		x /= v.x; y /= v.y; z /= v.z; return *this;
	}

	inline Vector3& operator+=(float f)
	{
		x += f; y += f; z += f; return *this;
	}

	inline Vector3& operator-=(float f)
	{
		x -= f; y -= f; z -= f; return *this;
	}

	inline Vector3& operator*=(float f)
	{
		x *= f; y *= f; z *= f; return *this;
	}

	inline Vector3& operator/=(float v)
	{
		x /= v; y /= v; z /= v; return *this;
	}

	inline Vector3 operator-() const
	{
		return Vector3(-x, -y, -z);
	}

	inline Vector3 operator+(const Vector3& v) const
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	inline Vector3 operator-(const Vector3& v) const
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	inline Vector3 operator*(const Vector3& v) const
	{
		return Vector3(x * v.x, y * v.y, z * v.z);
	}

	inline Vector3 operator/(const Vector3& v) const
	{
		return Vector3(x / v.x, y / v.y, z / v.z);
	}

	inline Vector3 operator+(float v) const
	{
		return Vector3(x + v, y + v, z + v);
	}

	inline Vector3 operator-(float v) const
	{
		return Vector3(x - v, y - v, z - v);
	}

	inline Vector3 operator*(float v) const
	{
		return Vector3(x * v, y * v, z * v);
	}

	inline Vector3 operator/(float v) const
	{
		return Vector3(x / v, y / v, z / v);
	}

	inline friend std::ostream& operator<<(std::ostream& os, const Vector3& v)
	{
		os << "X: " << v.x << " Y: " << v.y << " Z: " << v.z << std::endl;
		return os;
	}

	inline float Length() const
	{
		return fastsqrtf(x * x + y * y + z * z);
	}

	inline float LengthSqr() const
	{
		return (x * x + y * y + z * z);
	}

	inline float LengthXY() const
	{
		return fastsqrtf(x * x + y * y);
	}

	inline float LengthXZ() const
	{
		return fastsqrtf(x * x + z * z);
	}

	inline float DistTo(const Vector3& v) const
	{
		return (*this - v).Length();
	}

	inline Vector3& Normalize()
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

	inline float Dot(const Vector3& v) const
	{
		return (x * v.x + y * v.y + z * v.z);
	}

	inline Vector3 Cross(const Vector3& v) const
	{
		return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}

	inline Vector2 XY() const
	{
		return Vector2(x, y);
	}

	inline Vector2 XZ() const
	{
		return Vector2(x, z);
	}

	inline Vector2 YZ() const
	{
		return Vector2(y, z);
	}

	inline bool IsZero() const
	{
		return (x == 0.0f && y == 0.0f && z == 0.0f);
	}

	inline bool IsZeroTolerance(float tolerance = 0.01f) const
	{
		return (x > -tolerance && x < tolerance
			&&	y > -tolerance && y < tolerance
			&&	z > -tolerance && z < tolerance);
	}
};

class Vector3Aligned : public Vector3
{
public:
	Vector3Aligned()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		w = 1.0f;
	}

	Vector3Aligned(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		w = 1.0f;
	}

	Vector3Aligned(float x, float y, float z, float w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	Vector3Aligned(const Vector3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = 1.0f;
	}

private:
	float w;
};

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
};

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

	Matrix4x4()
		: m0(_mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f)), m1(_mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f)),
		m2(_mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f)), m3(_mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f))
	{
	}

	Matrix4x4(const Matrix4x4& mOther)
	{
		*this = mOther;
	}

	Matrix4x4(const __m128& m0, const __m128& m1, const __m128& m2, const __m128& m3)
		: m0(m0), m1(m1), m2(m2), m3(m3)
	{
	}

	Matrix4x4(float _00, float _01, float _02, float _03,
		float _10, float _11, float _12, float _13,
		float _20, float _21, float _22, float _23,
		float _30, float _31, float _32, float _33)
		: _00(_00), _01(_01), _02(_02), _03(_03),
		_10(_10), _11(_11), _12(_12), _13(_13),
		_20(_20), _21(_21), _22(_22), _23(_23),
		_30(_30), _31(_31), _32(_32), _33(_33)
	{
	}

	inline void Transpose()
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

	inline Matrix4x4& operator*(const Matrix4x4& matrix)
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

	inline float* operator[](int i)
	{
		return this->m[i];
	}

	inline const float* operator[](int i) const
	{
		return (const float*)this->m[i];
	}

	inline Vector3& Transform(const Vector3& v)
	{
		return Vector3(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3], m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3], m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3]);
	}

	inline Vector3& GetAxis(int i)
	{
		return *(Vector3*)((*this)[i]);
	}

	inline friend std::ostream& operator<<(std::ostream& os, const Matrix4x4 m)
	{
		os << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << ", " << m[0][3] << std::endl
			<< m[1][0] << ", " << m[1][1] << ", " << m[1][2] << ", " << m[1][3] << std::endl
			<< m[2][0] << ", " << m[2][1] << ", " << m[2][2] << ", " << m[2][3] << std::endl
			<< m[3][0] << ", " << m[3][1] << ", " << m[3][2] << ", " << m[3][3] << std::endl;

		return os;
	}
};
typedef Matrix4x4 VMatrix;

class Matrix3x4
{
public:
	union
	{
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
		};

		float m[3][4];
		float mm[12];
	};

	inline Vector3& GetAxis(int i)
	{
		return *(Vector3*)&m[i][0];
	}

	float* operator[](int i)
	{
		if (i < 0 || i > 3)
			return nullptr;

		return m[i];
	}

	const float* operator[](int i) const
	{
		if (i < 0 || i > 3)
			return nullptr;

		return m[i];
	}

	float* Base()
	{
		return &m[0][0];
	}

	const float* Base() const
	{
		return &m[0][0];
	}
};

enum
{
	PITCH = 0, // up / down
	YAW, // left / right
	ROLL // fall over
};

class Math
{
public:
	static Math* Get()
	{
		static Math* Instance = new Math();
		return Instance;
	}

	static void inline SinCos(float radians, float* sine, float* cosine)
	{
		*sine = sin(radians);
		*cosine = cos(radians);
	}

	static void AngleVectors(const Vector3& angles, Vector3* forward)
	{

		float sp, sy, cp, cy;

		SinCos(DEGTORAD(angles[PITCH]), &sp, &cp);
		SinCos(DEGTORAD(angles[YAW]), &sy, &cy);

		if (forward)
		{
			forward->x = cp * cy;
			forward->y = cp * sy;
			forward->z = -sp;
		}
	}

	static void AngleVectors(const Vector3& angles, Vector3* forward, Vector3* right, Vector3* up)
	{

		float sr, sp, sy, cr, cp, cy;

		SinCos(DEGTORAD(angles[PITCH]), &sp, &cp);
		SinCos(DEGTORAD(angles[YAW]), &sy, &cy);
		SinCos(DEGTORAD(angles[ROLL]), &sr, &cr);

		if (forward)
		{
			forward->x = cp * cy;
			forward->y = cp * sy;
			forward->z = -sp;
		}

		if (right)
		{
			right->x = (-1 * sr*sp*cy + -1 * cr*-sy);
			right->y = (-1 * sr*sp*sy + -1 * cr*cy);
			right->z = -1 * sr*cp;
		}

		if (up)
		{
			up->x = (cr*sp*cy + -sr * -sy);
			up->y = (cr*sp*sy + -sr * cy);
			up->z = cr * cp;
		}
	}

	static float DotProduct(Vector3 &v1, float* v2)
	{
		return v1.x*v2[0] + v1.y*v2[1] + v1.z*v2[2];
	}

	static float Dot(const Vector3 &v1, Vector3 &v2)
	{
		return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
	}

	static void VectorTransform(Vector3 &in1, Matrix3x4& in2, Vector3 &out)
	{
		out.x = DotProduct(in1, in2.m[0]) + in2.m[0][3];
		out.y = DotProduct(in1, in2.m[1]) + in2.m[1][3];
		out.z = DotProduct(in1, in2.m[2]) + in2.m[2][3];
	}

	static float VecLength(Vector3& vec)
	{
		return (float)fastsqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	}

	static float VecDist(Vector3& fVec1, Vector3& fVec2)
	{
		return (float)fastsqrtf(pow(fVec1.x - fVec2.x, 2) + pow(fVec1.y - fVec2.y, 2) + pow(fVec1.z - fVec2.z, 2));
	}

	static float GetFov(Vector3 angle, Vector3 src, Vector3 dst)
	{
		Vector3 ang, aim;
		ang = CalcAngle(src, dst);
		MakeVector(angle, aim);
		MakeVector(ang, ang);

		float mag = aim.LengthSqr();
		float u_dot_v = Dot(aim, ang);

		return RADTODEG(acos(u_dot_v / mag));
	}

	static Vector3 CalcAngle(Vector3 PlayerPos, Vector3 EnemyPos)
	{
		Vector3 AimAngles;
		Vector3 delta = PlayerPos - EnemyPos;
		float hyp = (float)sqrtf((delta.x * delta.x) + (delta.y * delta.y)); //SUPER SECRET IMPROVEMENT CODE NAME DONUT STEEL
		AimAngles.x = atanf(delta.z / hyp) * M_RADPI;
		AimAngles.y = atanf(delta.y / delta.x) * M_RADPI;
		AimAngles.z = 0.0f;

		if (delta.x >= 0.0)
			AimAngles.y += 180.0f;

		return AimAngles;
	}

	static void VectorAngles(const Vector3& dir, Vector3 &angles)
	{
		float hyp = (float)fastsqrtf((dir.x * dir.x) + (dir.y * dir.y)); //SUPER SECRET IMPROVEMENT CODE NAME DONUT STEEL
		angles.x = atanf(dir.z / hyp) * M_RADPI;
		angles.y = atanf(dir.y / dir.x) * M_RADPI;
		angles.z = 0.0f;
		if (dir.x >= 0.0f)
			angles.y += 180.0f;
	}

	static void ClampAngle(Vector3& angles)
	{
		if (angles.x < -89.0f)
			angles.x = 89.0f;
		if (angles.x > 89.0f)
			angles.x = 89.0f;
		if (angles.y < -180.0f)
			angles.y += 360.0f;
		if (angles.y > 180.0f)
			angles.y -= 360.0f;
		if (angles.z != 0.0f)
			angles.z = 0.0f;
	}

	static void VectorNormalize(Vector3& v)
	{
		float l = VecLength(v);
		if (l != 0.0f)
		{
			v /= l;
		}
		else
		{
			v.x = v.y = 0.0f; v.z = 1.0f;
		}
	}

	static void SmoothAngle(Vector3& ViewAngle, Vector3& DestAngles, float smooth)
	{
		Vector3 vecDelta = ViewAngle - DestAngles;
		ClampAngle(vecDelta);
		DestAngles = ViewAngle - vecDelta / 100.0f * smooth; // 50.0f is ur smooth value
	}

	static void MakeVector(const Vector3& angle, Vector3& vector)
	{
		float pitch = float(angle[0] * M_PI / 180);
		float yaw = float(angle[1] * M_PI / 180);
		float tmp = float(cos(pitch));
		vector[0] = float(-tmp * -cos(yaw));
		vector[1] = float(sin(yaw)*tmp);
		vector[2] = float(-sin(pitch));
	}

	static Vector3 AngleToDirection(Vector3 angle)
	{
		// Convert angle to radians 
		angle.x = DEGTORAD(angle.x);
		angle.y = DEGTORAD(angle.y);

		float sinYaw = (float)sin(angle.y);
		float cosYaw = (float)cos(angle.y);

		float sinPitch = (float)sin(angle.x);
		float cosPitch = (float)cos(angle.x);

		Vector3 direction;
		direction.x = cosPitch * cosYaw;
		direction.y = cosPitch * sinYaw;
		direction.z = -sinPitch;

		return direction;
	}

	static void VectorITransform(Vector3& in1, const Matrix3x4& in2, Vector3& out)
	{
		float in1t[3];

		in1t[0] = in1.x - in2.m[0][3];
		in1t[1] = in1.y - in2.m[1][3];
		in1t[2] = in1.z - in2.m[2][3];

		out.x = in1t[0] * in2.m[0][0] + in1t[1] * in2.m[1][0] + in1t[2] * in2.m[2][0];
		out.y = in1t[0] * in2.m[0][1] + in1t[1] * in2.m[1][1] + in1t[2] * in2.m[2][1];
		out.z = in1t[0] * in2.m[0][2] + in1t[1] * in2.m[1][2] + in1t[2] * in2.m[2][2];
	}

	static void VectorIRotate(Vector3& in1, const Matrix3x4& in2, Vector3& out)
	{
		out.x = in1.x * in2.m[0][0] + in1.y * in2.m[1][0] + in1.z * in2.m[2][0];
		out.y = in1.x * in2.m[0][1] + in1.y * in2.m[1][1] + in1.z * in2.m[2][1];
		out.z = in1.x * in2.m[0][2] + in1.y * in2.m[1][2] + in1.z * in2.m[2][2];
	}
};