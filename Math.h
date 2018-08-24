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

	static float GetFov(Vector3& angle, Vector3& src, Vector3& dst)
	{
		Vector3 ang, aim;
		ang = CalcAngle(src, dst);
		MakeVector(angle, aim);
		MakeVector(ang, ang);

		float mag = aim.LengthSqr();
		float u_dot_v = Dot(aim, ang);

		return RADTODEG(acos(u_dot_v / mag));
	}

	static Vector3 CalcAngle(Vector3& PlayerPos, Vector3& EnemyPos)
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

	static Vector3 AngleToDirection(Vector3& angle)
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