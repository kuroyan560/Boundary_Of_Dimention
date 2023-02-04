#pragma once
#include"KuroEngine.h"

namespace KuroEngine
{
	//èƒuƒŒ
	class HandShake
	{
		static const int SEED_MIN = 0;
		static const int SEED_MAX = 100;
		Vec3<float>m_offset = { 0,0,0 };
		float m_t;
		Vec2<int>m_seed = { 0,1 };

	public:
		float m_interval = 300.0f;
		Vec2<float>m_val = { 1.0f,1.0f };

		HandShake() {}
		void Init();
		void Update(float arg_timeScale, const Matrix& arg_rotate);

		//ƒuƒŒ—Ê
		const Vec3<float>& GetOffset()const { return m_offset; }
		//¬”“_•”•ª
		float GetTimeRate()const { return m_t - static_cast<float>(static_cast<int>(m_t)); }
	};
}