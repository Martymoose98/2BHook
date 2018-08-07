#pragma once

#include <Windows.h>
#include <xmmintrin.h>
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

EXTERN_C double inline __fastcall fastsqrt(double n);
EXTERN_C float inline __fastcall fastsqrtf(float n);

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

	inline Vector3& operator+=(float v)
	{
		x += v; y += v; z += v; return *this;
	}

	inline Vector3& operator-=(float v)
	{
		x -= v; y -= v; z -= v; return *this;
	}

	inline Vector3& operator*=(float v)
	{
		x *= v; y *= v; z *= v; return *this;
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
		return sqrtf(x * x + y * y + z * z);
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

	inline bool IsZero() const
	{
		return (x == 0.0f && y == 0.0f && z == 0.0f);
	}

	inline bool _IsZero(float tolerance = 0.01f) const
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
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};

		float m[4][4];
		float mm[16];

		struct
		{
			__m128 m1, m2, m3, m4;
		};
	};

	inline void Transpose()
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				m[i][j] = m[j][i];
			}
		}
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

	inline float* operator[](int i) const
	{
		return (float*)&this->m[i];
	}

	inline Vector3& Transform(const Vector3& v)
	{
		return Vector3(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3], m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3], m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3]);
	}

	inline Vector3& GetAxis(int i)
	{
		return *(Vector3*)&this->m[i][0]; //((*this)[i]);
	}
};

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


	//void inline SinCos(float radians, float* sine, float* cosine)
	//{
	//	__asm
	//	{
	//		fld	DWORD PTR[radians]
	//		fsincos

	//		mov edx, DWORD PTR[cosine]
	//		mov eax, DWORD PTR[sine]

	//		fstp DWORD PTR[edx]
	//		fstp DWORD PTR[eax]
	//	}
	//}

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
			forward->x = cp*cy;
			forward->y = cp*sy;
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
			forward->x = cp*cy;
			forward->y = cp*sy;
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
			up->x = (cr*sp*cy + -sr*-sy);
			up->y = (cr*sp*sy + -sr*cy);
			up->z = cr*cp;
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