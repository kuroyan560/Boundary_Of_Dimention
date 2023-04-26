#pragma once
#include"Common/Singleton.h"
#include"ForUser/TimeScale.h"
class TimeScaleMgr : public KuroEngine::DesignPattern::Singleton<TimeScaleMgr>
{
	friend class KuroEngine::DesignPattern::Singleton<TimeScaleMgr>;
	TimeScaleMgr() {}

	//�Q�[�����I�u�W�F�N�g�̎���
	KuroEngine::TimeScale m_inGame;

public:
	const float& GetInGameTimeScale()const { return m_inGame.GetTimeScale(); }
};

