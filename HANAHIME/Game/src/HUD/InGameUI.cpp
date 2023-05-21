#include "InGameUI.h"
#include"../OperationConfig.h"

std::list<InGameUI*>InGameUI::s_inGameUiList;
KuroEngine::Timer InGameUI::s_noOpeTimer;
KuroEngine::Timer InGameUI::s_initIdleTimer;

void InGameUI::Init()
{
	const float INIT_IDLE_TIME = 300.0f;
	s_initIdleTimer.Reset(INIT_IDLE_TIME);
}

void InGameUI::Update(float arg_timeScale)
{
	const bool operationInput = OperationConfig::Instance()->CheckAllOperationInput();
	const float NO_OPERATION_TIME = 120.0f;

	bool isAllAppeared = true;
	bool isAllDisappeared = true;
	for (auto& ui : s_inGameUiList)
	{
		if (!ui->IsAppeared())isAllAppeared = false;
		if (!ui->IsDisappeared())isAllDisappeared = false;
	}

	if (isAllAppeared)
	{
		if (s_initIdleTimer.IsTimeUp() && operationInput)
		{
			for (auto& ui : s_inGameUiList)ui->Disappear();
			s_noOpeTimer.Reset(NO_OPERATION_TIME);
		}

		s_initIdleTimer.UpdateTimer(arg_timeScale);

		if (s_initIdleTimer.IsTimeUpOnTrigger())
		{
			for (auto& ui : s_inGameUiList)ui->Disappear();
			s_noOpeTimer.Reset(NO_OPERATION_TIME);
		}
	}
	else if (isAllDisappeared)
	{
		if (operationInput)s_noOpeTimer.Reset(NO_OPERATION_TIME);

		if (s_noOpeTimer.UpdateTimer(arg_timeScale))
		{
			for (auto& ui : s_inGameUiList)ui->Appear();
		}
	}
}
