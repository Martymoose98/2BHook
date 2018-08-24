#pragma once
#include "Math.h"

#define RGBA_COLOR32(r, g, b, a) (int)(((BYTE)(a) << 24) | ((BYTE)(b) << 16) | ((BYTE)(g) << 8) | (BYTE)(r))
#define RGBX_COLOR32(r, g, b) (int)(((BYTE)(0xFF) << 24) | ((BYTE)(b) << 16) | ((BYTE)(g) << 8) | (BYTE)(r))

class Color
{
public:
	Color()
		: m_color32(0)
	{
	}

	Color(int color32)
		: m_color32(color32)
	{
	}

	Color(int _r, int _g, int _b)
	{
		SetColor(_r, _g, _b);
	}

	Color(int _r, int _g, int _b, int _a)
	{
		SetColor(_r, _g, _b, _a);
	}

	Color(const Vector4& vColor)
	{
		SetColor((int)(vColor[0] * 255.0f), (int)(vColor[1] * 255.0f), (int)(vColor[2] * 255.0f), (int)(vColor[3] * 255.0f));
	}

	Color(const float* rgba)
	{
		SetColor(rgba[0] * 255.0f, rgba[1] * 255.0f, rgba[2] * 255.0f, rgba[3] * 255.0f);
	}

	void SetColor(int _r, int _g, int _b, int _a = 255)
	{
		m_color[0] = (unsigned char)_r;
		m_color[1] = (unsigned char)_g;
		m_color[2] = (unsigned char)_b;
		m_color[3] = (unsigned char)_a;
	}

	void GetColor(int& _r, int& _g, int& _b, int& _a) const
	{
		_r = m_color[0];
		_g = m_color[1];
		_b = m_color[2];
		_a = m_color[3];
	}

	const unsigned char* RetColor() const
	{
		return m_color;
	}

	void SetRawColor(int color32)
	{
		this->m_color32 = color32;
	}

	int GetRawColor() const
	{
		return this->m_color32;
	}

	int GetD3DColor() const
	{
		return ((int)(((m_color[3]) << 24) | ((m_color[0]) << 16) | ((m_color[1]) << 8) | (m_color[2])));
	}

	inline int r() const
	{
		return m_color[0];
	}

	inline int g() const
	{
		return m_color[1];
	}

	inline int b() const
	{
		return m_color[2];
	}

	inline int a() const
	{
		return m_color[3];
	}

	inline float rBase() const
	{
		return m_color[0] / 255.0f;
	}

	inline float gBase() const
	{
		return m_color[1] / 255.0f;
	}

	inline float bBase() const
	{
		return m_color[2] / 255.0f;
	}

	inline float aBase() const
	{
		return m_color[3] / 255.0f;
	}

	unsigned char& operator[](int index)
	{
		return m_color[index];
	}

	const unsigned char& operator[](int index) const
	{
		return m_color[index];
	}

	bool operator==(const Color& rhs) const
	{
		return (m_color32 == rhs.m_color32);
	}

	bool operator!=(const Color& rhs) const
	{
		return !(operator==(rhs));
	}

	Color& operator=(const Color& rhs)
	{
		SetRawColor(rhs.GetRawColor());
		return *this;
	}

	operator Vector4()
	{
		return Vector4(BaseAlpha());
	}

	operator XMFLOAT4()
	{
		return XMFLOAT4(BaseAlpha());
	}

	void GetFloatColor(float* rgba) const
	{
		rgba[0] = m_color[0] / 255.0f;
		rgba[1] = m_color[1] / 255.0f;
		rgba[2] = m_color[2] / 255.0f;
		rgba[3] = m_color[3] / 255.0f;
	}

	__forceinline float* Base() const
	{
		float clr[3];

		clr[0] = m_color[0] / 255.0f;
		clr[1] = m_color[1] / 255.0f;
		clr[2] = m_color[2] / 255.0f;

		return &clr[0];
	}

	__forceinline float* BaseAlpha() const
	{
		float clr[4];

		clr[0] = m_color[0] / 255.0f;
		clr[1] = m_color[1] / 255.0f;
		clr[2] = m_color[2] / 255.0f;
		clr[3] = m_color[3] / 255.0f;

		return &clr[0];
	}

	float Hue() const
	{
		if (m_color[0] == m_color[1] && m_color[1] == m_color[2])
		{
			return 0.0f;
		}

		float r = m_color[0] / 255.0f;
		float g = m_color[1] / 255.0f;
		float b = m_color[2] / 255.0f;

		float max = r > g ? r : g > b ? g : b,
			min = r < g ? r : g < b ? g : b;
		float delta = max - min;
		float hue = 0.0f;

		if (r == max)
		{
			hue = (g - b) / delta;
		}
		else if (g == max)
		{
			hue = 2 + (b - r) / delta;
		}
		else if (b == max)
		{
			hue = 4 + (r - g) / delta;
		}
		hue *= 60;

		if (hue < 0.0f)
		{
			hue += 360.0f;
		}
		return hue;
	}

	float Saturation() const
	{
		float r = m_color[0] / 255.0f;
		float g = m_color[1] / 255.0f;
		float b = m_color[2] / 255.0f;

		float max = r > g ? r : g > b ? g : b,
			min = r < g ? r : g < b ? g : b;
		float l, s = 0;

		if (max != min)
		{
			l = (max + min) / 2;
			if (l <= 0.5f)
				s = (max - min) / (max + min);
			else
				s = (max - min) / (2 - max - min);
		}
		return s;
	}

	float Brightness() const
	{
		float r = m_color[0] / 255.0f;
		float g = m_color[1] / 255.0f;
		float b = m_color[2] / 255.0f;

		float max = r > g ? r : g > b ? g : b;
		float min = r < g ? r : g < b ? g : b;

		return (max + min) / 2;
	}

	static Color FromHSB(float hue, float saturation, float brightness)
	{
		float h = (hue == 1.0f) ? 0 : hue * 6.0f;
		float f = h - (int)h;
		float p = brightness * (1.0f - saturation);
		float q = brightness * (1.0f - saturation * f);
		float t = brightness * (1.0f - (saturation * (1.0f - f)));

		if (h < 1)
		{
			return Color(
				(unsigned char)(brightness * 255),
				(unsigned char)(t * 255),
				(unsigned char)(p * 255)
			);
		}
		else if (h < 2)
		{
			return Color(
				(unsigned char)(q * 255),
				(unsigned char)(brightness * 255),
				(unsigned char)(p * 255)
			);
		}
		else if (h < 3)
		{
			return Color(
				(unsigned char)(p * 255),
				(unsigned char)(brightness * 255),
				(unsigned char)(t * 255)
			);
		}
		else if (h < 4)
		{
			return Color(
				(unsigned char)(p * 255),
				(unsigned char)(q * 255),
				(unsigned char)(brightness * 255)
			);
		}
		else if (h < 5)
		{
			return Color(
				(unsigned char)(t * 255),
				(unsigned char)(p * 255),
				(unsigned char)(brightness * 255)
			);
		}
		else
		{
			return Color(
				(unsigned char)(brightness * 255),
				(unsigned char)(p * 255),
				(unsigned char)(q * 255)
			);
		}
	}

	static Color Red()
	{
		return Color(255, 0, 0);
	}

	static Color Green()
	{
		return Color(0, 255, 0);
	}

	static Color Blue()
	{
		return Color(0, 0, 255);
	}

	static Color Yellow()
	{
		return Color(255, 255, 0);
	}

	static Color LightBlue()
	{
		return Color(100, 100, 255);
	}

	static Color Grey()
	{
		return Color(128, 128, 128);
	}

	static Color DarkGrey()
	{
		return Color(45, 45, 45);
	}

	static Color Black()
	{
		return Color(0, 0, 0);
	}

	static Color White()
	{
		return Color(255, 255, 255);
	}

	static Color Purple()
	{
		return Color(220, 0, 220);
	}

private:
	union
	{
		unsigned char m_color[4];
		unsigned int m_color32;

		struct
		{
			unsigned char m_r;
			unsigned char m_g;
			unsigned char m_b;
			unsigned char m_a;
		};
	};
};