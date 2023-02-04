#pragma once
#include"Common/Vec.h"
#include"ForUser/Timer.h"

namespace KuroEngine
{
	//衝撃による揺れ
	class ImpactShake
	{
		//アクティブフラグ
		bool m_active = false;
		//振動の総時間タイマー
		Timer m_activeTimer;
		//振動のスパン計測タイマー
		Timer m_spanTimer;
		//振動の強さ乱数下限
		float m_powerMin;
		//振動の強さ乱数上限
		float m_powerMax;
		//振動オフセット
		Vec3<float>m_offset;

		//オフセットに乗算
		Vec3<float>m_mul = { 1.0f,1.0f,1.0f };

	public:
		ImpactShake(Vec3<float>arg_mul) :m_mul(arg_mul) {}
		void Init()
		{
			m_offset = { 0,0,0 };
			m_activeTimer.Reset(0);
			m_active = false;
		}
		void Update(float arg_timeScale);

		//オフセットゲッタ
		const Vec3<float>& GetOffset()const { return m_offset; }

		//振動
		void Shake(float arg_time, float arg_span, float arg_powerMin, float arg_powerMax);

		//アクティブフラグゲッタ
		const bool& IsActive()const { return m_active; }
	};
}