#pragma once
#include"ForUser/Timer.h"
#include<memory>
#include"Common/Vec.h"
#include"Common/Angle.h"

namespace KuroEngine
{
	class TextureBuffer;
}

class CameraModeUI
{
	//アクティブ状態
	bool m_isActive = false;

	//出現中
	bool m_isAppear = true;

	//アイコン画像
	std::shared_ptr<KuroEngine::TextureBuffer>m_icon;

	//描画位置
	KuroEngine::Vec2<float>m_pos;

	//アルファ
	float m_alpha;

	//時間計測
	KuroEngine::Timer m_timer;

	//リサージュ曲線レート
	KuroEngine::Angle m_lissajousAngle = 0.0f;

public:
	CameraModeUI();

	void Init() { m_isActive = false; }
	void Update(float arg_timeRate);
	void Draw();

	void Appear()
	{
		if (m_isAppear)return;
		if (m_isActive)
		{
			m_isAppear = true;
			return;
		}

		m_isActive = true;
		m_isAppear = true;
		m_lissajousAngle = 0.0f;
		m_timer.Reset(55.0f);
	}

	void Disappear()
	{
		m_isAppear = false;
		m_timer.Reset(55.0f);
	}
};

