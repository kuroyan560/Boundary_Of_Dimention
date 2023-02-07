#pragma once
#include"KuroEngine.h"
#include"Common/Singleton.h"
class DebugController : public KuroEngine::DesignPattern::Singleton<DebugController>
{
	friend class KuroEngine::DesignPattern::Singleton<DebugController>;
	DebugController();

	bool m_active = false;

public:
	void Update();

	//‹N“®’†‚©
	const bool& IsActive()const { return m_active; }
};

