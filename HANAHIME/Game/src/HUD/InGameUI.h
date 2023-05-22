#pragma once
#include<list>
#include"ForUser/Timer.h"
class InGameUI
{
private:
	static std::list<InGameUI*>s_inGameUiList;
	static KuroEngine::Timer s_noOpeTimer;
	static KuroEngine::Timer s_initIdleTimer;

	virtual void Appear() = 0;
	virtual void Disappear() = 0;
	virtual bool IsAppeared() = 0;
	virtual bool IsDisappeared() = 0;
public:
	InGameUI()
	{
		s_inGameUiList.push_front(this);
	}

	static void Init();
	static void Update(float arg_timeScale);
};