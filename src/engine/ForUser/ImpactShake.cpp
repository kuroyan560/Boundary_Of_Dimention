#include "ImpactShake.h"
#include"KuroEngine.h"

void KuroEngine::ImpactShake::Update(float arg_timeScale)
{
	if (!m_active)return;

	//振動中
	if (!m_activeTimer.UpdateTimer(arg_timeScale))
	{
		//振動スパン計測
		if (m_spanTimer.UpdateTimer(arg_timeScale))
		{
			//時間が経つほど振動が弱くなっていく
			float timeRate = m_activeTimer.GetTimeRate();
			float shakePowerScale = KuroEngine::Math::Ease(Out, Circ, timeRate, 1.0f, 0.0f);

			//振動オフセット値を求める
			m_offset.x = GetRand(m_powerMin, m_powerMax)
				* shakePowerScale * GetRandPlusMinus();
			m_offset.y = GetRand(m_powerMin, m_powerMax)
				* shakePowerScale * GetRandPlusMinus();
			m_offset.z = GetRand(m_powerMin, m_powerMax)
				* shakePowerScale * GetRandPlusMinus();

			m_spanTimer.Reset();
		}
	}
	else
	{
		//振動終わり
		m_offset = { 0,0,0 };
		m_active = false;
	}
}

void KuroEngine::ImpactShake::Shake(float arg_time, float arg_span, float arg_powerMin, float arg_powerMax)
{
	//アクティブ状態に
	m_active = true;

	//各種振動パラメータセット
	m_activeTimer.Reset(arg_time);
	m_spanTimer.Reset(arg_span);
	m_powerMin = arg_powerMin;
	m_powerMax = arg_powerMax;

	//振動オフセット初期値計算
	m_offset.x = GetRand(arg_powerMin, arg_powerMax) * GetRandPlusMinus();
	m_offset.y = GetRand(arg_powerMin, arg_powerMax) * GetRandPlusMinus();
	m_offset.z = GetRand(arg_powerMin, arg_powerMax) * GetRandPlusMinus();
}
