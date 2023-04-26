#pragma once
#include"Common/Singleton.h"
#include"ForUser/TimeScale.h"
class TimeScaleMgr : public KuroEngine::DesignPattern::Singleton<TimeScaleMgr>
{
	friend class KuroEngine::DesignPattern::Singleton<TimeScaleMgr>;
	TimeScaleMgr() {}

	//ゲーム内オブジェクトの時間
	KuroEngine::TimeScale m_inGame;

public:
	const float& GetInGameTimeScale()const { return m_inGame.GetTimeScale(); }
};

