#pragma once
#include"Common/Singleton.h"

class CheckPointHitFlag : public KuroEngine::DesignPattern::Singleton<CheckPointHitFlag> {

public:

	bool m_isHitCheckPointTrigger = false;


};