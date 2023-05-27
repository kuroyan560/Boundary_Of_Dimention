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
	//�A�N�e�B�u���
	bool m_isActive = false;

	//�o����
	bool m_isAppear = true;

	//�A�C�R���摜
	std::shared_ptr<KuroEngine::TextureBuffer>m_icon;

	//�`��ʒu
	KuroEngine::Vec2<float>m_pos;

	//�A���t�@
	float m_alpha;

	//���Ԍv��
	KuroEngine::Timer m_timer;

	//���T�[�W���Ȑ����[�g
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

