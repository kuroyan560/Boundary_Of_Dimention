#pragma once
#include"Common/Singleton.h"
class GateManager : public KuroEngine::DesignPattern::Singleton<GateManager>
{
	bool m_enterGate = false;
	int m_destStageNum = -1;
	int m_destGateID = -1;
	bool m_nonTouch = false;
	int m_touchCount = 0;

	friend class KuroEngine::DesignPattern::Singleton<GateManager>;
	GateManager() {}
public:
	void Init()
	{
		m_enterGate = false;
		m_destStageNum = -1;
		m_destGateID = -1;
		m_nonTouch = false;
		m_touchCount = 0;
	}
	void FrameEnd();
	void SetEnter(bool arg_enter, int arg_destStageNum, int arg_destGateID);
	const bool& IsEnter()const { return m_enterGate; }
	const int& GetDestStageNum()const { return m_destStageNum; }
	const int& GetDestGateID()const { return m_destGateID; }
};