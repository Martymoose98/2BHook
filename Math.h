#pragma once
#include <Windows.h>
#include <DirectXMath.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <ostream>
#include <time.h>
#include <math.h>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"

#define M_RADPI 57.295779513082f
#define M_PI 3.14159265358979323846
#define M_PI_F		((float)(M_PI))

#define SQUARE(a) (a) * (a)
#define CUBE(a) (a) * (a) * (a)
#define DEGTORAD(x)  ((float)(x) * (float)( M_PI_F / 180.f ))
#define RADTODEG(x)  ((float)(x) * (float)( 180.f / M_PI_F ))

using DirectX::XMFLOAT4;

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

double __forceinline loga(double a, double x)
{
	return log(x) / log(a);
}

float __forceinline logaf(float a, float x)
{
	return logf(x) / logf(a);
}

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

enum eTransformMatrix
{
	RIGHT,
	UP,
	FORWARD,
	POSITION
};

class Math
{
public:

	static void inline SinCos(float radians, float* sine, float* cosine)
	{
		*sine = sin(radians);
		*cosine = cos(radians);
	}

	static void AngleVectors(const Vector3& angles, Vector3* forward)
	{
		float sp, sy, cp, cy;

		SinCos(angles[PITCH], &sp, &cp);
		SinCos(angles[YAW], &sy, &cy);

		if (forward)
		{
			forward->x = cp * sy;
			forward->y = sp;
			forward->z = cp * cy;
		}
	}

	static void AngleVectors(const Vector3& angles, Vector3* forward, Vector3* right, Vector3* up)
	{

		float sr, sp, sy, cr, cp, cy;

		SinCos(angles[PITCH], &sp, &cp);
		SinCos(angles[YAW], &sy, &cy);
		SinCos(angles[ROLL], &sr, &cr);

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

	static float Dot(const Vector3& v1, const float* v2)
	{
		return v1.x * v2[0] + v1.y * v2[1] + v1.z * v2[2];
	}

	static float Dot(const Vector3& v1, const Vector3& v2)
	{
		return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
	}

	/*
	 theta = acos(v dot u / |v| * |u|)
	*/
	static float GetFov(const Vector3& vForward, const Vector3& src, const Vector3& dst)
	{
		Vector3 ang;
		CalcAngle(src, dst, ang);
		AngleVectors(ang, &ang);
		return RADTODEG(acos(vForward.Dot(ang) / (vForward.Length() * ang.Length())));
	}

	static inline void CalcAngle(const Vector3& src, const Vector3& dst, Vector3& angles)
	{
		VectorAngles((dst - src).Normalize(), angles);
	}

	static inline void VectorAngles(const Vector3& dir, Vector3& angles)
	{
		float hyp = ssesqrt((dir.x * dir.x) + (dir.z * dir.z));
		angles.x = atanf(dir.y / hyp);
		angles.y = atanf(dir.x / dir.z);
		angles.z = 0.0f;

		if (dir.z >= 0.0f)
			angles.y += M_PI_F;
	}

	static void ClampAngle(Vector3& angles)
	{
		if (angles.x < -DEGTORAD(89.0f))
			angles.x = DEGTORAD(89.0f);
		if (angles.x > DEGTORAD(89.0f))
			angles.x = DEGTORAD(89.0f);
		if (angles.y < DEGTORAD(-180.0f))
			angles.y += DEGTORAD(360.0f);
		if (angles.y > DEGTORAD(180.0f))
			angles.y -= DEGTORAD(360.0f);
		if (angles.z != 0.0f)
			angles.z = 0.0f;
	}

	static void VectorNormalize(Vector3& v)
	{
		float l = v.Length();

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
		float pitch = DEGTORAD(angle[PITCH]);
		float yaw = DEGTORAD(angle[YAW]);
		float cp = cos(pitch);
		vector[0] = cp * cos(yaw);
		vector[1] = cp * sin(yaw);
		vector[2] = -sin(pitch);
	}

	static void VectorITransform(const Vector3& in1, const Matrix3x4& in2, Vector3& out)
	{
		float in1t[3];

		in1t[0] = in1.x - in2[0][3];
		in1t[1] = in1.y - in2[1][3];
		in1t[2] = in1.z - in2[2][3];

		out.x = in1t[0] * in2[0][0] + in1t[1] * in2[1][0] + in1t[2] * in2[2][0];
		out.y = in1t[0] * in2[0][1] + in1t[1] * in2[1][1] + in1t[2] * in2[2][1];
		out.z = in1t[0] * in2[0][2] + in1t[1] * in2[1][2] + in1t[2] * in2[2][2];
	}

	static void VectorIRotate(const Vector3& in1, const Matrix3x4& in2, Vector3& out)
	{
		out.x = in1.x * in2.m[0][0] + in1.y * in2.m[1][0] + in1.z * in2.m[2][0];
		out.y = in1.x * in2.m[0][1] + in1.y * in2.m[1][1] + in1.z * in2.m[2][1];
		out.z = in1.x * in2.m[0][2] + in1.y * in2.m[1][2] + in1.z * in2.m[2][2];
	}

	static void VectorTransform(const Vector3& in1, const Matrix3x4& in2, Vector3& out)
	{
		out.x = Dot(in1, in2.m[0]) + in2.m[0][3];
		out.y = Dot(in1, in2.m[1]) + in2.m[1][3];
		out.z = Dot(in1, in2.m[2]) + in2.m[2][3];
	}

	static void VectorRotate(const Vector3& in1, const Matrix4x4& in2, Vector3& out)
	{
		out.y = in1.Dot(in2[UP]);
		out.x = in1.Dot(in2[RIGHT]);
		out.z = in1.Dot(in2[FORWARD]);
	}
};