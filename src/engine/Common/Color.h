#pragma once
#include<vector>

namespace KuroEngine
{
	struct Color
	{
	public:
		float m_r = 1.0f;
		float m_g = 1.0f;
		float m_b = 1.0f;
		float m_a = 1.0f;

		//コンストラクタ
		Color() {};
		Color(const Color& Color) :m_r(Color.m_r), m_g(Color.m_g), m_b(Color.m_b), m_a(Color.m_a) {}
		Color(float R, float G, float B, float A) : m_r(R), m_g(G), m_b(B), m_a(A) {}
		Color(int R, int G, int B, int A) {
			m_r = R / 255.0f;
			m_g = G / 255.0f;
			m_b = B / 255.0f;
			m_a = A / 255.0f;
		}

		//比較演算子
		bool operator==(const Color& rhs) {
			if (m_r != rhs.m_r)return false;
			if (m_g != rhs.m_g)return false;
			if (m_b != rhs.m_b)return false;
			if (m_a != rhs.m_a)return false;
			return true;
		}
		bool operator!=(const Color& rhs) {
			return !(*this == rhs);
		}

		//代入演算子
		void operator=(const Color& rhs)
		{
			m_r = rhs.m_r;
			m_g = rhs.m_g;
			m_b = rhs.m_b;
			m_a = rhs.m_a;
		}
	};
}